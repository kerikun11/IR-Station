# IR-Station Firmware

## 開発環境

- [PlatformIO - Espressif32](https://docs.platformio.org/en/latest/platforms/espressif32.html)
  - VSCode: 拡張機能 `PlatformIO` をインストール
  - コマンドライン開発: `pip install platformio`

### 設定

設定ファイル `platformio.ini` でシリアルポートを設定する (例: `/dev/ttyUSB0`, `COM3`)

```ini
upload_port = /dev/ttyUSB0
monitor_port = /dev/ttyUSB0
```

### 実行例

```sh
# firmware をビルドする (VSCode: Ctrl+Alt+B)
platformio run
# firmware を書き込む (VSCode: Ctrl+Alt+U)
platformio run -t upload
```
