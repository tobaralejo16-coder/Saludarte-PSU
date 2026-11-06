package com.example.saludarteapp

import android.Manifest
import android.content.Context
import android.database.ContentObserver
import android.media.AudioManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.rounded.DirectionsWalk
import androidx.compose.material.icons.filled.Bluetooth
import androidx.compose.material.icons.filled.BluetoothDisabled
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material.icons.rounded.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.saludarteapp.ui.theme.*
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.rememberMultiplePermissionsState
import kotlin.math.roundToInt

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        val bleManager = BleManager(this)
        
        enableEdgeToEdge()
        setContent {
            SaludarteAppTheme {
                SaludarteHomeScreen(bleManager)
            }
        }
    }
}

data class ActivityItem(
    val id: String,
    val name: String,
    val color: Color,
    val icon: ImageVector
)

@OptIn(ExperimentalMaterial3Api::class, ExperimentalPermissionsApi::class)
@Composable
fun SaludarteHomeScreen(bleManager: BleManager) {
    val context = LocalContext.current
    val connectionState by bleManager.connectionState.collectAsState()
    
    // Control de Volumen del Celular (Sincronización en segundo plano con la ESP32)
    val audioManager = remember { context.getSystemService(Context.AUDIO_SERVICE) as AudioManager }
    val maxVolume = remember { audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC).coerceAtLeast(1) }
    var currentVolume by remember { mutableIntStateOf(audioManager.getStreamVolume(AudioManager.STREAM_MUSIC)) }

    val volumeHandler = remember { Handler(Looper.getMainLooper()) }
    var lastVolSentTime by remember { mutableLongStateOf(0L) }

    fun sendVolumeToEsp32(volume: Int) {
        val level = (volume.toFloat() / maxVolume.toFloat() * 9f).roundToInt().coerceIn(0, 9)
        val now = System.currentTimeMillis()
        volumeHandler.removeCallbacksAndMessages(null)
        val delay = (150L - (now - lastVolSentTime)).coerceAtLeast(0L)
        volumeHandler.postDelayed({
            if (connectionState == BleManager.BleState.CONNECTED) {
                bleManager.sendMessage("V$level")
                lastVolSentTime = System.currentTimeMillis()
            }
        }, delay)
    }

    // Escuchar cambios de volumen físicos del celular
    DisposableEffect(context) {
        val observer = object : ContentObserver(Handler(Looper.getMainLooper())) {
            override fun onChange(selfChange: Boolean) {
                val newVol = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC)
                if (newVol != currentVolume) {
                    currentVolume = newVol
                    sendVolumeToEsp32(newVol)
                }
            }
        }
        context.contentResolver.registerContentObserver(
            Settings.System.CONTENT_URI,
            true,
            observer
        )
        onDispose {
            context.contentResolver.unregisterContentObserver(observer)
            volumeHandler.removeCallbacksAndMessages(null)
        }
    }

    // Al conectarse al ESP32, sincronizar el volumen actual
    LaunchedEffect(connectionState) {
        if (connectionState == BleManager.BleState.CONNECTED) {
            sendVolumeToEsp32(currentVolume)
        }
    }

    val bluetoothPermissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        listOf(
            Manifest.permission.BLUETOOTH_SCAN,
            Manifest.permission.BLUETOOTH_CONNECT
        )
    } else {
        listOf(
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.BLUETOOTH,
            Manifest.permission.BLUETOOTH_ADMIN
        )
    }
    
    val permissionState = rememberMultiplePermissionsState(permissions = bluetoothPermissions)

    val activities = listOf(
        ActivityItem("01", "Despertar", PastelYellow, Icons.Rounded.WbSunny),
        ActivityItem("02", "Desayunar", PastelGreen, Icons.Rounded.Restaurant),
        ActivityItem("03", "Bañarse", PastelBlue, Icons.Rounded.Bathtub),
        ActivityItem("04", "Caminata", PastelTeal, Icons.AutoMirrored.Rounded.DirectionsWalk),
        ActivityItem("05", "Merendar", PastelOrange, Icons.Rounded.BakeryDining),
        ActivityItem("06", "Almorzar", PastelGreen, Icons.Rounded.LunchDining),
        ActivityItem("07", "Cenar", PastelIndigo, Icons.Rounded.DinnerDining),
        ActivityItem("08", "Taller", PastelPurple, Icons.Rounded.Brush),
        ActivityItem("09", "Dormir", PastelIndigo, Icons.Rounded.Bedtime),
        ActivityItem("10", "Medicamento", PastelPink, Icons.Rounded.MedicalServices),
        ActivityItem("11", "Cepillarse", PastelBlue, Icons.Rounded.CleanHands),
        ActivityItem("12", "Descanso", PastelYellow, Icons.Rounded.SelfImprovement)
    )

    Scaffold(
        modifier = Modifier.fillMaxSize()
    ) { innerPadding ->
        // Fondo Difuminado Azul Clarito
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(
                    Brush.verticalGradient(
                        colors = listOf(
                            Color(0xFFE3F2FD), // Azul clarito
                            Color.White
                        )
                    )
                )
                .padding(innerPadding)
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                // Bluetooth Button (Top-Right)
                Box(modifier = Modifier.fillMaxWidth()) {
                    IconButton(
                        onClick = {
                            if (permissionState.allPermissionsGranted) {
                                if (connectionState == BleManager.BleState.DISCONNECTED) {
                                    bleManager.startScan()
                                } else {
                                    bleManager.disconnect()
                                }
                            } else {
                                permissionState.launchMultiplePermissionRequest()
                            }
                        },
                        modifier = Modifier.align(Alignment.TopEnd)
                    ) {
                        Icon(
                            imageVector = if (connectionState == BleManager.BleState.CONNECTED) 
                                Icons.Default.Bluetooth else Icons.Default.BluetoothDisabled,
                            contentDescription = "Bluetooth Status",
                            tint = if (connectionState == BleManager.BleState.CONNECTED) 
                                SaludarteGreenPrimary else Color.Gray
                        )
                    }
                }

                // Header: Logo Centrado + Letrero
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    modifier = Modifier.padding(bottom = 16.dp)
                ) {
                    Image(
                        painter = painterResource(id = R.drawable.sin_fondo),
                        contentDescription = "Logo Saludarte",
                        modifier = Modifier
                            .size(140.dp)
                            .padding(bottom = 4.dp)
                    )
                    Text(
                        text = "SALUDARTE",
                        fontSize = 32.sp,
                        fontWeight = FontWeight.Black,
                        color = SaludarteGreenPrimary,
                        letterSpacing = 4.sp,
                        textAlign = TextAlign.Center
                    )
                    
                    // Status Indicator debajo del título
                    val statusText = when (connectionState) {
                        BleManager.BleState.DISCONNECTED -> "Desconectado"
                        BleManager.BleState.SCANNING -> "Buscando ESP32..."
                        BleManager.BleState.CONNECTING -> "Conectando..."
                        BleManager.BleState.CONNECTED -> "Conectado"
                    }
                    Surface(
                        shape = RoundedCornerShape(16.dp),
                        color = Color.White.copy(alpha = 0.5f),
                        modifier = Modifier.padding(top = 6.dp)
                    ) {
                        Text(
                            text = statusText,
                            style = MaterialTheme.typography.bodySmall,
                            color = if (connectionState == BleManager.BleState.CONNECTED) SaludarteGreenPrimary else Color.Gray,
                            modifier = Modifier.padding(horizontal = 12.dp, vertical = 4.dp)
                        )
                    }
                }

                Text(
                    text = "Actividades Diarias",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier.padding(bottom = 12.dp),
                    color = SaludarteGreenPrimary.copy(alpha = 0.6f)
                )

                LazyVerticalGrid(
                    columns = GridCells.Fixed(2),
                    modifier = Modifier.weight(1f),
                    verticalArrangement = Arrangement.spacedBy(16.dp),
                    horizontalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    items(activities) { activity ->
                        ActivityButton(activity) {
                            if (connectionState == BleManager.BleState.CONNECTED) {
                                bleManager.sendMessage(activity.id)
                                Toast.makeText(context, "Enviando: ${activity.name}", Toast.LENGTH_SHORT).show()
                            } else {
                                Toast.makeText(context, "No conectado al ESP32", Toast.LENGTH_SHORT).show()
                            }
                        }
                    }
                }

                Spacer(modifier = Modifier.height(16.dp))

                // Botón de Emergencia Destacado
                Button(
                    onClick = {
                        if (connectionState == BleManager.BleState.CONNECTED) {
                            bleManager.sendMessage("99")
                            Toast.makeText(context, "¡EMERGENCIA ENVIADA!", Toast.LENGTH_LONG).show()
                        } else {
                            Toast.makeText(context, "No conectado al ESP32", Toast.LENGTH_SHORT).show()
                        }
                    },
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(72.dp),
                    colors = ButtonDefaults.buttonColors(containerColor = EmergencyRed),
                    shape = RoundedCornerShape(20.dp),
                    elevation = ButtonDefaults.buttonElevation(defaultElevation = 8.dp)
                ) {
                    Icon(
                        Icons.Default.Warning,
                        contentDescription = null,
                        modifier = Modifier.size(32.dp),
                        tint = Color.White
                    )
                    Spacer(modifier = Modifier.width(12.dp))
                    Text(
                        text = "EMERGENCIA",
                        fontSize = 22.sp,
                        fontWeight = FontWeight.Black,
                        color = Color.White
                    )
                }
            }
        }
    }
}

@Composable
fun ActivityButton(activity: ActivityItem, onClick: () -> Unit) {
    Card(
        onClick = onClick,
        modifier = Modifier
            .fillMaxWidth()
            .height(125.dp),
        colors = CardDefaults.cardColors(containerColor = activity.color),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp),
        shape = RoundedCornerShape(24.dp)
    ) {
        Column(
            modifier = Modifier.fillMaxSize().padding(12.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Icon(
                imageVector = activity.icon,
                contentDescription = null,
                modifier = Modifier.size(44.dp),
                tint = SaludarteGreenPrimary.copy(alpha = 0.7f)
            )
            Spacer(modifier = Modifier.height(6.dp))
            Text(
                text = activity.name,
                color = SaludarteGreenPrimary,
                fontSize = 15.sp,
                fontWeight = FontWeight.Bold,
                textAlign = TextAlign.Center
            )
        }
    }
}

@Preview(showBackground = true)
@Composable
fun SaludartePreview() {
    SaludarteAppTheme {
        SaludarteHomeScreen(BleManager(LocalContext.current))
    }
}
