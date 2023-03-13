# Retro-Printer-Gate
Alduino shield for USB Serial/UART TTL to Old Centoronics printer port 

Rev.2023/Mar/13

USBシリアル→セントロだけテスト済みです。  
TTLシリアル→セントロはテスト中です。  
セントロ→USB/TTLシリアルはテスト環境が用意できていないので無効化しています。  

※動作確認のため試験的に公開状態にしています。
テスト済みの箇所でも、環境によっては動かない場合があるかもしれません。

DIPSW
1 ON --- OFF USB-SERIAL
2 ignore
3 4 ON ON   9600bps
3 4 ON OFF  19200bps
3 4 OFF ON  57600bps
3 4 OFF OFF 115200bps

