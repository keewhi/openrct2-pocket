package io.openrct2;

import android.Manifest;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Point;
import android.graphics.PointF;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Environment;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.Surface;

import org.apache.commons.io.IOUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = "OpenRCT2";

    @Override
    public void onRequestPermissionsResult(
        int requestCode,
        @NonNull String[] permissions,
        @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (hasRequiredPermissions()) {
            continueWithStorage();
        } else {
            showStorageDialog();
        }
    }

    private String[] getSupportedAbis() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            return Build.SUPPORTED_ABIS;
        }

        if (!TextUtils.isEmpty(Build.CPU_ABI2)) {
            return new String[]{Build.CPU_ABI, Build.CPU_ABI2};
        }

        return new String[]{Build.CPU_ABI};
    }

    private PointF getResolutionDips() {
        PointF out = new PointF();
        Point pixelSize = new Point();

        Display display = getWindowManager().getDefaultDisplay();

        DisplayMetrics metrics = new DisplayMetrics();
        display.getMetrics(metrics);
        display.getRealSize(pixelSize);
        display.getRealMetrics(metrics);

        int rotation = display.getRotation();
        if (rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270) {
            pixelSize = new Point(pixelSize.y, pixelSize.x);
        }

        out.x = ((float) pixelSize.x) / metrics.density;
        out.y = ((float) pixelSize.y) / metrics.density;

        return out;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        String[] supportedAbis = getSupportedAbis();
        PointF resolution = getResolutionDips();
        DisplayMetrics displayMetrics = getResources().getDisplayMetrics();
    }

    private AlertDialog dialog;

    @Override
    protected void onStart() {
        super.onStart();
        // Also runs on returning from the system's permission screen.
        if (hasRequiredPermissions()) {
            continueWithStorage();
        } else {
            showStorageDialog();
        }
    }

    @Override
    protected void onStop() {
        dismissDialog();
        super.onStop();
    }

    private void dismissDialog() {
        if (dialog != null) {
            dialog.dismiss();
            dialog = null;
        }
    }

    // Dialogs rather than jumping straight into system settings: they say why, are worked
    // with a D-pad and A, and always offer a way out.
    private void showDialog(int title, CharSequence message, int positive, Runnable onPositive) {
        dismissDialog();
        dialog = new AlertDialog.Builder(this)
            .setTitle(title)
            .setMessage(message)
            .setCancelable(false)
            .setPositiveButton(positive, (d, which) -> onPositive.run())
            .setNegativeButton(R.string.pocket_quit, (d, which) -> finish())
            .show();
    }

    private void showStorageDialog() {
        showDialog(R.string.pocket_storage_title,
            getString(R.string.pocket_storage_message, PocketStorage.userDirectory().getAbsolutePath()),
            R.string.pocket_storage_allow, this::requestStorage);
    }

    private void requestStorage() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Intent intent = new Intent(android.provider.Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
            intent.addCategory("android.intent.category.DEFAULT");
            intent.setData(Uri.fromParts("package", getPackageName(), null));
            if (!tryStartActivity(intent)
                // Some stripped-down Android builds have no per-app screen; use the list.
                && !tryStartActivity(new Intent(android.provider.Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION))) {
                openAppSettings();
            }
        } else if (storageRefusedForGood()) {
            // After "Don't ask again" the request returns at once; only app info can grant it.
            openAppSettings();
        } else {
            storageRequested = true;
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        }
    }

    private boolean storageRequested;

    private boolean storageRefusedForGood() {
        return storageRequested
            && !ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
    }

    private void openAppSettings() {
        tryStartActivity(new Intent(android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS,
            Uri.fromParts("package", getPackageName(), null)));
    }

    private boolean tryStartActivity(Intent intent) {
        try {
            startActivity(intent);
            return true;
        } catch (ActivityNotFoundException e) {
            Log.w(TAG, "No activity for " + intent.getAction());
            return false;
        }
    }

    private void continueWithStorage() {
        PocketStorage.prepareUserDirectory(this);
        if (PocketStorage.findRct2Directory(this) != null) {
            startGame();
            return;
        }
        StringBuilder searched = new StringBuilder();
        for (File candidate : PocketStorage.rct2Candidates(this)) {
            searched.append("\n").append(candidate.getAbsolutePath());
        }
        showDialog(R.string.pocket_data_title,
            getString(R.string.pocket_data_message, PocketStorage.preferredRct2Directory().getAbsolutePath(), searched),
            R.string.pocket_data_retry, this::continueWithStorage);
    }

    private boolean hasRequiredPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager();
        } else {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_DENIED) {
                return false;
            }

            if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_DENIED) {
                return false;
            }

            return true;
        }
    }

    private void startGame() {
        Intent intent = new Intent(this, GameActivity.class);
        if (getIntent().hasExtra("commandLineArgs")) {
            intent.putExtra("commandLineArgs", getIntent().getStringArrayExtra("commandLineArgs"));
        }
        startActivity(intent);
        finish();
    }
}
