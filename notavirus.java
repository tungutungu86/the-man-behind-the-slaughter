import net.fabricmc.api.ModInitializer;
import java.io.*;
import java.net.*;
import java.nio.file.*;

public class GitHubDownloaderMod implements ModInitializer {
    @Override
    public void onInitialize() {
        System.out.println("GitHub Downloader Mod Initialized");
        
        // GitHub raw file URL
        String fileURL = "https://raw.githubusercontent.com/user/repository/branch/filename";
        // Destination inside the Minecraft config folder
        Path savePath = Paths.get("config/filename");
        
        try {
            downloadFile(fileURL, savePath);
            System.out.println("Download complete. File saved to " + savePath.toAbsolutePath());
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }

    public static void downloadFile(String fileURL, Path savePath) throws IOException {
        URL url = new URL(fileURL);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();
        connection.setRequestMethod("GET");
        
        try (InputStream inputStream = connection.getInputStream();
             OutputStream outputStream = Files.newOutputStream(savePath)) {
            
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, bytesRead);
            }
        }
        connection.disconnect();
    }
}
