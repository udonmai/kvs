kvs ストア
============================

kvs ストア.
kvs ストア用に作ったライブラリ hash.c はシンプルながらも
3秒程度で 1000万件, 1GB のテキスト情報がメモリに乗り O(1) で検索できる.

kvs-server は hash.c のネットワーク用ラッパー.
memcached 互換プロトコル実装（一部未実装のプロトコルあり）

インストール方法
----------------------------

    $ make

使い方
----------------------------

    $ ./kvs-server &
    $ ./kvs-client
    connecting to server localhost
    
    kvs > help
    set <key> <value> ... <key> <value>の値を保存する.
    get <key> ... <key> に対応する値を取得する
    delete <key> ... <key> に対応する値を削除する
    find <key> ... <key> が存在するかどうかを確認する
    purge ... キャッシュを削除する
    help ... このメッセージを出力する
    status  ... サーバーの状態を表示する
    quit ... サーバーとの接続を切る
    
    kvs > set 5 foor
    OK. 5 - foor
    
    kvs > get 5
    (key, value): (5, foor)
    
    kvs > get 6   
    6 : no value
    
    kvs > status    
    uptime        ...    247.20 sec
    port number   ...      9877
    key numbers   ...         2
    get cmd       ...         2
    set cmd       ...         2
    delete cmd    ...         0
    find cmd      ...         0
    memory alloc  ...        78
    
    kvs > quit
