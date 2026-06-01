.PHONY: all clean build-front uploadfs upload deploy monitor run

# Default target: build frontend, compile firmware, and upload everything
all: deploy

# Build the React PWA dashboard (Vite production build)
build-front:
	cd dashboard && npm run build

# Clean the PlatformIO build cache
clean:
	pio run -t clean

# Build and upload the LittleFS filesystem image (web assets) to the ESP32
uploadfs: build-front
	pio run -t uploadfs

# Build and upload the C++ firmware binary to the ESP32
upload:
	pio run -t upload

# Open the PlatformIO serial device monitor
monitor:
	pio device monitor

# Deploy everything and immediately open the serial monitor to view CPU logs
run: deploy monitor

# Deploy everything cleanly: clean build, compile React assets, flash LittleFS, and flash firmware
deploy: clean build-front uploadfs upload
	@echo ==========================================================
	@echo  Deployment Complete! Real-time node is live on Core 1.
	@echo ==========================================================
