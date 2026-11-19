package io.openrct2;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

/**
 * Where OpenRCT2 Pocket keeps its files on Android.
 *
 * Saves and settings live in their own folder, so Pocket can sit beside upstream OpenRCT2
 * (which uses /sdcard/openrct2-user) without the two sharing a config.ini. The player's own
 * RCT2 files are looked for in Pocket's folder first and then in upstream's /sdcard/rct2, on
 * the internal storage and on any SD card.
 */
final class PocketStorage {
    private static final String TAG = "OpenRCT2 Pocket";
    static final String USER_FOLDER = "openrct2-pocket";
    private static final String DEFAULT_CONFIG_ASSET = "pocket/config.ini";

    private PocketStorage() {
    }

    static File userDirectory() {
        return new File(Environment.getExternalStorageDirectory(), USER_FOLDER);
    }

    /** Where the player is told to copy their RCT2 files. */
    static File preferredRct2Directory() {
        return new File(userDirectory(), "RCT2");
    }

    /** Roots of the internal storage and every mounted SD card, internal first. */
    private static List<File> storageRoots(Context context) {
        Set<File> roots = new LinkedHashSet<>();
        roots.add(Environment.getExternalStorageDirectory());
        File[] appDirectories = context.getExternalFilesDirs(null);
        if (appDirectories != null) {
            for (File appDirectory : appDirectories) {
                if (appDirectory == null) {
                    continue; // A volume that is not mounted right now.
                }
                // <root>/Android/data/<package>/files
                String path = appDirectory.getAbsolutePath();
                int android = path.indexOf("/Android/");
                if (android > 0) {
                    roots.add(new File(path.substring(0, android)));
                }
            }
        }
        return new ArrayList<>(roots);
    }

    /** Every folder that is checked for RCT2 files, in order. */
    static List<File> rct2Candidates(Context context) {
        List<File> candidates = new ArrayList<>();
        for (File root : storageRoots(context)) {
            candidates.add(new File(new File(root, USER_FOLDER), "RCT2"));
            candidates.add(new File(root, "rct2"));
            candidates.add(new File(root, "RCT2"));
        }
        return candidates;
    }

    /** The first folder holding the player's RCT2 files, or null. */
    static File findRct2Directory(Context context) {
        for (File candidate : rct2Candidates(context)) {
            if (hasRct2Data(candidate)) {
                return candidate;
            }
        }
        return null;
    }

    /** True when the folder contains Data/g1.dat. Copies from Windows can differ in case. */
    static boolean hasRct2Data(File directory) {
        File data = childIgnoringCase(directory, "Data");
        return data != null && childIgnoringCase(data, "g1.dat") != null;
    }

    private static File childIgnoringCase(File directory, String name) {
        File exact = new File(directory, name);
        if (exact.exists()) {
            return exact;
        }
        String[] names = directory.list();
        if (names == null) {
            return null;
        }
        for (String candidate : names) {
            if (candidate.equalsIgnoreCase(name)) {
                return new File(directory, candidate);
            }
        }
        return null;
    }

    /**
     * Creates the user folder and, on first launch, the handheld settings (the same role the
     * NextUI launcher's config.ini plays on the Brick). An existing config is never replaced.
     */
    static void prepareUserDirectory(Context context) {
        File userDirectory = userDirectory();
        if (!userDirectory.isDirectory() && !userDirectory.mkdirs()) {
            Log.w(TAG, "Could not create " + userDirectory);
            return;
        }
        File config = new File(userDirectory, "config.ini");
        if (config.exists()) {
            return;
        }
        try (InputStream in = context.getAssets().open(DEFAULT_CONFIG_ASSET);
             OutputStream out = new FileOutputStream(config)) {
            byte[] buffer = new byte[4096];
            int read;
            while ((read = in.read(buffer)) > 0) {
                out.write(buffer, 0, read);
            }
        } catch (IOException e) {
            Log.w(TAG, "Could not write the default config.ini", e);
            //noinspection ResultOfMethodCallIgnored
            config.delete();
        }
    }
}
