~/arduino-ide/arduino-cli compile -j 8 --fqbn esp32:esp32:esp32c3 \
  --build-property "build.partitions=min_spiffs" \
  --build-property "upload.maximum_size=3145728" \
  -v -e ~/Arduino/Girlianda/
