package com.example.saludarteapp.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext

private val DarkColorScheme = darkColorScheme(
    primary = SaludarteGreenSecondary,
    secondary = SaludarteBlueSecondary,
    tertiary = EmergencyRed,
    background = BackgroundDark,
    surface = BackgroundDark
)

private val LightColorScheme = lightColorScheme(
    primary = SaludarteGreenPrimary,
    secondary = SaludarteBluePrimary,
    tertiary = EmergencyRed,
    background = BackgroundLight,
    surface = Color.White
)

@Composable
fun SaludarteAppTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    // Set to false by default to prefer our custom branding colors
    dynamicColor: Boolean = false,
    content: @Composable () -> Unit
) {
    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            val context = LocalContext.current
            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }

        darkTheme -> DarkColorScheme
        else -> LightColorScheme
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}