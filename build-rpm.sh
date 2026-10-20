#!/bin/bash
set -euo pipefail

IMAGE_NAME="same-gnome-rpm-builder"
OUTPUT_DIR="$(pwd)/rpm-output"

echo "==> Building Docker image..."
docker build -f Dockerfile.rpm -t "$IMAGE_NAME" .

echo "==> Creating output directory..."
mkdir -p "$OUTPUT_DIR"

echo "==> Running RPM build in container..."
docker run --rm -v "$OUTPUT_DIR:/output" "$IMAGE_NAME"

echo "==> RPM packages built successfully:"
ls -lh "$OUTPUT_DIR"/*.rpm 2>/dev/null || echo "No RPM files found in $OUTPUT_DIR"