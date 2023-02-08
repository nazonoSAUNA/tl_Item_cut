# 切り取りプラグイン
拡張編集の「切り取り」が特殊な為、皆が思うような切り取りを実装
リップル切り取りも実装

[ダウンロードはこちら](../../releases/)

## 動作要件
- Visual C++ 再頒布可能パッケージの2022 x86(32bit)対応の物がインストールされている必要があります【Microsoft Visual C++ 2015-2022 Redistributable(x86)】→ 導入方法:< https://scrapbox.io/nazosauna/Visual_C++_再頒布可能パッケージをインストールする >

- 拡張編集バージョン0.92のみ対応しています


## 仕様
- ショートカットに[切り取り][切り取り]・[切り取り][リップル]・[切り取り][TLリップル]を追加します

![image](https://user-images.githubusercontent.com/99536641/217500435-f4c6988d-86e8-4ffa-97c3-5ecc06e13789.png)

- [切り取り][切り取り]の内容としては切り取りキーが押されたとき、拡張編集に[コピー]と[削除]の命令を出すだけです
- [切り取り][リップル]は頑張ってリップル切り取りを実装しています（バグがある可能性あり）
- [切り取り][TLリップル]は選択オブジェクトを範囲として拡張編集の「切り取り」を実行します

- 以下を選択した状態で各切り取りを行った時の挙動を示す

![cut1](https://user-images.githubusercontent.com/99536641/217500789-abf8dfa0-5280-44d9-919d-6da92cd01824.png)

- 切り取り後

![cut2](https://user-images.githubusercontent.com/99536641/217500796-2efa4a5a-a069-4211-b882-8600c182b936.png)

![cut3](https://user-images.githubusercontent.com/99536641/217500799-7c6bf265-4719-4734-8e24-c8423ada5f85.png)

![cut4](https://user-images.githubusercontent.com/99536641/217500802-a564ccd1-4e95-4250-941f-17232b4770f4.png)

![cut5](https://user-images.githubusercontent.com/99536641/217500805-040a2d2a-952e-459e-a700-fe27995524cc.png)
