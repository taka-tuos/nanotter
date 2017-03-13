# nanotter
twitter at CUI.
とりあえずalpha版ということで。  
C99出身C99育ちですので脆弱性とかわかんないです。  
あったらissueに投げていただけると嬉しいです。  
Power for ておくれ。  
そのうちluaプラグイン実装します。  

##必須ライブラリ
OpenSSL  
libcurlw  
libjson  
libcurl  

##ビルド時の注意
twitcurlは付属のものを使ってください。
(Streaming APIに独自対応(超乱暴)させてます)

##ビルド方法
$ git clone https://github.com/taka-tuos/nanotter.git  
$ cd nanotter/twitcurl  
$ make && sudo make install  
$ cd ..  
$ make  


##今のところは
ツイートとTL閲覧だけです。  
ふぁぼふぁぼしたりリツイートしたりツイ消ししたりはできません。  
※プラグインもまだです  

##つかいかた
nanotterを端末で起動して上に出た > にコマンド入力ですべての操作を行います。  
helpって打てばコマンド一覧が出ます。  
