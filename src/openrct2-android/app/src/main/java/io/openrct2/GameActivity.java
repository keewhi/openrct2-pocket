package io.openrct2;

import android.annotation.SuppressLint;
import android.icu.util.Currency;
import android.icu.util.LocaleData;
import android.icu.util.ULocale;
import android.os.Build;
import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;
import android.util.Log;
import android.view.View;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

public class GameActivity extends SDLActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Read by the native code (and by SDL, for hints) when the game starts, so these must
        // be set before SDLActivity loads the libraries and starts the game thread.
        setEnvironment("OPENRCT2_HANDHELD", "1");
        // The system Back key reaches the game as a key (Pocket treats it as B) instead of
        // closing the activity behind the game's back.
        setEnvironment("SDL_ANDROID_TRAP_BACK_BUTTON", "1");
        // Crisp pixels when the canvas is scaled to the panel.
        setEnvironment("SDL_RENDER_SCALE_QUALITY", "0");
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onDestroy() {
        // SDLActivity waits here for the game thread to end (quitting saves the park first).
        super.onDestroy();
        if (isFinishing()) {
            // The native code keeps its state in statics, which a later launch in this same
            // process would inherit half-torn-down. Start every launch from a fresh process.
            System.exit(0);
        }
    }

    private static void setEnvironment(String name, String value) {
        try {
            Os.setenv(name, value, true);
        } catch (ErrnoException e) {
            Log.w(MainActivity.TAG, "Could not set " + name, e);
        }
    }

    public float getDefaultScale() {
        return getResources().getDisplayMetrics().density;
    }

    @SuppressLint("ObsoleteSdkInt")
    private Locale getDeviceLocale() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return getResources().getConfiguration().getLocales().get(0);
        } else {
            return getResources().getConfiguration().locale;
        }
    }
    public String getDefaultLocale(String[] supportedTags) {
        Locale deviceLocale = getDeviceLocale();

        for (String supportedTag : supportedTags) {
            if (supportedTag.isEmpty()) continue;
            String[] splits = supportedTag.split("-");
            String language = splits[0];
            String country = splits[1];
            if (deviceLocale.getLanguage().equals(language) && deviceLocale.getCountry().equals(country)) {
                return supportedTag;
            }
        }

        Locale canadaEn = Locale.CANADA;
        if (canadaEn.getLanguage().equals(deviceLocale.getLanguage()) && canadaEn.getCountry().equals(deviceLocale.getCountry())) {
            return "en-US";
        }

        for (String supportedTag : supportedTags) {
            if (supportedTag.isEmpty()) continue;
            String[] splits = supportedTag.split("-");
            String language = splits[0];
            if (deviceLocale.getLanguage().equals(language)) {
                return supportedTag;
            }
        }
        return "en-UK";
    }

    public String getLocaleCurrency() {
        Locale deviceLocale = getDeviceLocale();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return Currency.getInstance(deviceLocale).getCurrencyCode();
        } else {
            return java.util.Currency.getInstance(deviceLocale).getCurrencyCode();
        }
    }

    public boolean isImperialLocaleMeasurementFormat() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            return LocaleData.getMeasurementSystem(ULocale.forLocale(getResources().getConfiguration().getLocales().get(0))) == LocaleData.MeasurementSystem.US;
        }
        Locale deviceLocale = getDeviceLocale();
        String localeCountry = deviceLocale.getCountry();
        return localeCountry.equals(Locale.US.getCountry()) || localeCountry.equals(new Locale("xx", "LR").getCountry()) || localeCountry.equals(new Locale("xx", "MM").getCountry());
    }

    @Override
    protected String[] getLibraries() {
        return new String[]{
                "c++_shared",
                "bz2",
                "freetype",
                "z",
                "png16",
                "SDL2",
                "icudata",
                "icuuc",
                "icui18n",

                "openrct2",
                "openrct2-ui"
        };
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        // Set app to fullscreen mode
        if (hasFocus) {
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            );
        }
    }

    protected String[] getArguments() {
        List<String> arguments = new ArrayList<>();
        arguments.add("--user-data-path=" + PocketStorage.userDirectory().getAbsolutePath());
        File rct2 = PocketStorage.findRct2Directory(this);
        if (rct2 != null) {
            arguments.add("--rct2-data-path=" + rct2.getAbsolutePath());
        }
        // Arguments passed in by whoever launched us come last, so they can override ours.
        if (getIntent().hasExtra("commandLineArgs")) {
            String[] extra = getIntent().getStringArrayExtra("commandLineArgs");
            if (extra != null) {
                arguments.addAll(Arrays.asList(extra));
            }
        }
        return arguments.toArray(new String[0]);
    }
}
