# 切り取りプラグイン
拡張編集の「切り取り」が特殊な為、皆が思うような切り取りを実装

## 動作要件
- Visual C++ 再頒布可能パッケージの2022 x86(32bit)対応の物がインストールされている必要があります【Microsoft Visual C++ 2015-2022 Redistributable(x86)】
- マイクロソフト公式:< https://docs.microsoft.com/ja-jp/cpp/windows/latest-supported-vc-redist >
- AviUtl解説サイト:< https://scrapbox.io/aviutl/Visual_C++_再頒布可能パッケージ >

- 拡張編集0.92のみ動作します

## 仕様
- ショートカットに[切り取り]を追加します

![image](https://user-images.githubusercontent.com/99536641/209899659-391c8b12-f8e2-4ef2-b77f-af1152b6b9be.png)

- 内容としては切り取りキーが押されたとき、拡張編集に[コピー]と[削除]の命令を出します
