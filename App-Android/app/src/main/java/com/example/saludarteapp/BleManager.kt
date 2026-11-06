package com.example.saludarteapp

import android.annotation.SuppressLint
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.os.Handler
import android.os.Looper
import android.os.ParcelUuid
import android.util.Log
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.util.*
import java.util.concurrent.ConcurrentLinkedQueue

@SuppressLint("MissingPermission")
class BleManager(private val context: Context) {

    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val manager = context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        manager.adapter
    }

    private var bluetoothGatt: BluetoothGatt? = null
    private var writeCharacteristic: BluetoothGattCharacteristic? = null

    private val _connectionState = MutableStateFlow(BleState.DISCONNECTED)
    val connectionState: StateFlow<BleState> = _connectionState

    // Cola de mensajes enviada de forma secuencial
    private val sendQueue = ConcurrentLinkedQueue<String>()
    @Volatile
    private var isWriting = false

    private val mainHandler = Handler(Looper.getMainLooper())

    enum class BleState {
        DISCONNECTED, SCANNING, CONNECTING, CONNECTED
    }

    companion object {
        const val SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
        const val CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
        const val DEVICE_NAME = "SALUDARTE_ESP32"
        private const val TAG = "BleManager"
    }

    fun startScan() {
        val adapter = bluetoothAdapter
        if (adapter == null || !adapter.isEnabled) {
            Log.e(TAG, "Bluetooth deshabilitado o no disponible")
            _connectionState.value = BleState.DISCONNECTED
            return
        }

        if (_connectionState.value != BleState.DISCONNECTED) return
        
        Log.d(TAG, "Iniciando escaneo de alta prioridad...")
        _connectionState.value = BleState.SCANNING
        
        val scanner = adapter.bluetoothLeScanner
        if (scanner == null) {
            Log.e(TAG, "No se pudo obtener el BluetoothLeScanner")
            _connectionState.value = BleState.DISCONNECTED
            return
        }

        val scanSettings = ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
            .build()

        val scanFilters = listOf(
            ScanFilter.Builder()
                .setServiceUuid(ParcelUuid(UUID.fromString(SERVICE_UUID)))
                .build(),
            ScanFilter.Builder()
                .setDeviceName(DEVICE_NAME)
                .build()
        )

        try {
            scanner.startScan(scanFilters, scanSettings, scanCallback)
        } catch (e: Exception) {
            Log.e(TAG, "Error iniciando escaneo filtrado, usando sin filtro: ${e.message}")
            try {
                scanner.startScan(scanCallback)
            } catch (ex: Exception) {
                Log.e(TAG, "Fallo al escanear: ${ex.message}")
            }
        }
        
        // Detener escaneo después de 12 segundos si no encuentra el dispositivo
        mainHandler.postDelayed({
            if (_connectionState.value == BleState.SCANNING) {
                stopScan()
                _connectionState.value = BleState.DISCONNECTED
            }
        }, 12000)
    }

    private fun stopScan() {
        try {
            bluetoothAdapter?.bluetoothLeScanner?.stopScan(scanCallback)
        } catch (e: Exception) {
            Log.e(TAG, "Error al detener escaneo: ${e.message}")
        }
    }

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val device = result.device
            val recordName = result.scanRecord?.deviceName
            val devName = device.name ?: recordName

            if (devName == DEVICE_NAME || result.scanRecord?.serviceUuids?.contains(ParcelUuid(UUID.fromString(SERVICE_UUID))) == true) {
                Log.d(TAG, "ESP32 Encontrado: ${device.address} (${devName})")
                stopScan()
                connectToDevice(device)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e(TAG, "Escaneo falló con código: $errorCode")
            _connectionState.value = BleState.DISCONNECTED
        }
    }

    private fun connectToDevice(device: BluetoothDevice) {
        _connectionState.value = BleState.CONNECTING
        
        mainHandler.post {
            @Suppress("DEPRECATION")
            bluetoothGatt = device.connectGatt(context, false, gattCallback, BluetoothDevice.TRANSPORT_LE)
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            Log.d(TAG, "onConnectionStateChange: status=$status, newState=$newState")
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "Conectado al servidor GATT.")
                _connectionState.value = BleState.CONNECTED
                mainHandler.postDelayed({
                    gatt.discoverServices()
                }, 300)
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d(TAG, "Desconectado del servidor GATT.")
                _connectionState.value = BleState.DISCONNECTED
                writeCharacteristic = null
                sendQueue.clear()
                isWriting = false
                try {
                    gatt.close()
                } catch (e: Exception) {
                    Log.e(TAG, "Error cerrando GATT", e)
                }
                bluetoothGatt = null
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                val service = gatt.getService(UUID.fromString(SERVICE_UUID))
                writeCharacteristic = service?.getCharacteristic(UUID.fromString(CHARACTERISTIC_UUID))
                Log.d(TAG, "Servicio y Característica descubiertos con éxito.")
                processNextInQueue()
            } else {
                Log.e(TAG, "Fallo al descubrir servicios: $status")
            }
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            Log.d(TAG, "onCharacteristicWrite: status=$status")
            isWriting = false
            mainHandler.postDelayed({
                processNextInQueue()
            }, 50)
        }
    }

    fun sendMessage(message: String) {
        if (_connectionState.value != BleState.CONNECTED) {
            Log.e(TAG, "No conectado. Mensaje descartado: $message")
            return
        }

        // Si es un mensaje de volumen "V...", descartar volúmenes obsoletos en la cola
        if (message.startsWith("V")) {
            sendQueue.removeIf { it.startsWith("V") }
        }

        sendQueue.add(message)
        processNextInQueue()
    }

    @Synchronized
    private fun processNextInQueue() {
        if (isWriting) return
        val gatt = bluetoothGatt ?: return
        val characteristic = writeCharacteristic ?: return
        
        val nextMessage = sendQueue.poll() ?: return

        isWriting = true
        characteristic.value = nextMessage.toByteArray()
        characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT

        val success = gatt.writeCharacteristic(characteristic)
        Log.d(TAG, "Enviando '$nextMessage', éxito inmediato=$success")

        if (!success) {
            isWriting = false
            mainHandler.postDelayed({
                processNextInQueue()
            }, 100)
        }
    }

    fun disconnect() {
        stopScan()
        sendQueue.clear()
        isWriting = false
        try {
            bluetoothGatt?.disconnect()
            bluetoothGatt?.close()
        } catch (e: Exception) {
            Log.e(TAG, "Error desconectando", e)
        }
        bluetoothGatt = null
        _connectionState.value = BleState.DISCONNECTED
    }
}
