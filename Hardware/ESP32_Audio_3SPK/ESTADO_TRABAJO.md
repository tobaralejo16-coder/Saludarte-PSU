# Entrega final disponible

27/09/2026: continuar desde ../ESP32_Audio_3SPK_FINAL/ESP32_Audio_3SPK.kicad_pro. F1 definitivo 2 A / 250 V / 5×20 mm; fuente 5 V / 2–3 A. Esta carpeta conserva la base de trabajo anterior; la BOM y documentación vigentes están en FINAL. Ambos ZIP finales se encuentran en outputs.

---

# Estado del proyecto — revisión 0.4

Actualizado: 27 de septiembre de 2026. Continuar sobre esta carpeta; revisión 0.3 conservada.

## Completado y guardado

- Edge.Cuts cerrado: 102 × 100 mm. Visor 3D sin error de contorno.
- Placement compacto y accesos revisados. ESP32 sobre dos sockets hembra THT; huella del ZIP idéntica byte por byte. Modelos asociados y orientación visual corregida.
- Ruteo refinado y derivación C2 acortada: 122 segmentos, 4 vías; sin esquinas de 90° entre dos segmentos. Troncal de 2,0–2,2 mm.
- Clearance global mínimo 0,25 mm guardado en el proyecto.
- Plano GND rellenado y conectado. Eliminada pista GND redundante; conservado solo puente necesario de U4.
- Alimentación a pin 19 de 5 V, tres salidas BTL y ganancia baja confirmadas sin cambiar conexiones.
- DRC normal: 0 errores, 0 warnings, 0 conexiones pendientes, 0 diferencias con esquema. ERC: 0 infracciones.
- Auditoría ampliada: seis avisos de filtros de biblioteca revisados y documentados; no indican error eléctrico. Ninguna exclusión añadida.
- Documentación, informes y vistas actualizados.

## Pendiente únicamente de validación física

- Medir ESP32 real, sockets y módulos de amplificador; verificar fijación, alturas y acceso a los bornes. Faltan modelos 3D completos de amplificadores.
- Seleccionar jack externo, fusible, interruptor y piezas definitivas. Confirmar cobre ≥35 µm para la estimación de pistas a 3 A.
- Ensayar alimentación/temperatura, límite de volumen de parlantes 8 Ω/1 W y señal I2S. Protocolo y almacenamiento de audio siguen sin definirse.

No fabricar sin cerrar esos puntos. No se generaron Gerbers. Estado CAD cerrado para esta revisión; no rehacer ni ejecutar los scripts antiguos de placement/ruteo.

Respaldo del estado al reanudar: `work/checkpoints/v04_2026-09-27/ESP32_Audio_3SPK_al_reanudar.kicad_pcb` (fuera de este paquete). SHA256: A761E78644C3A2D778619A7D66E5546F0C2935E118CCDAE5B87C32CEE04994B7.
