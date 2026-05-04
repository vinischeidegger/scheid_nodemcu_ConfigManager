# Data Files for ConfigManager

This directory contains files that will be uploaded to the device's file system (LittleFS/SPIFFS).

## Files

- **index.html** - Configuration portal UI served to users

## Deployment

To upload files to your device with PlatformIO:

1. Ensure the `data/` folder is at the project root (next to `platformio.ini`)
2. Run: `pio run -t uploadfs -e <environment>`

## Gzip Compression

Modern browsers support automatic gzip decompression. To optimize storage and bandwidth:

1. Pre-compress the HTML file to `index.html.gz` before uploading
2. The web server sends it with `Content-Encoding: gzip` header
3. Browsers automatically decompress it

### Manual Compression (Linux/macOS):
```bash
cd data
gzip -k index.html  # Creates index.html.gz, keeps original
```

### Manual Compression (Windows):
Use 7-Zip or similar tools to create `index.html.gz`

### Automatic Compression (Optional):
Add to your `platformio.ini` build environment to auto-compress during build (requires custom build script).

## File Size Impact

- Original index.html: ~5.5 KB
- Compressed index.html.gz: ~1.8 KB (68% reduction)

The ConfigManager will automatically fall back to inline HTML if the file is not found.
