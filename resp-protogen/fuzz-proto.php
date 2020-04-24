<?php

$obj_r = new Redis(); $obj_r->connect('127.0.0.1', 6379);
$obj_r2 = new Redis(); $obj_r->connect('127.0.0.1', 6379);

ini_set('default_socket_timeout', 9999);
$obj_r->setOption(Redis::OPT_READ_TIMEOUT, 9999);

$maxpct = $argv[1] ?? 50;

$on = 0;
++$on;

while (($arr = $obj_r->blpop("proto-files", 9999)) !== FALSE) {
    list ($key, $msg) = $arr;

    if ( ! is_file($msg)) {
        echo "[$on]:  File '$msg' doesn't exist, skipping!\n";
        continue;
    }

    $seed = time();
    $log = tempnam("/tmp/", "vg-protogen-");
    $pct = rand(1, $maxpct);
    $renew = rand(1, 2) == 1 ? "-r" : "";

    $str_cmd = "valgrind --log-file=$log --error-exitcode=1 --leak-check=full ./fail -qp $pct $renew -s $seed $msg";

    echo "[$on]: $str_cmd => ";
    exec("$str_cmd >/dev/null 2>&1", $arr_result, $exitcode);
    if ($exitcode != 0) {
        echo "FAIL\n";
        exit(-1);
    }

    unlink($log);
    unlink($msg);
    echo "OK\n";
    ++$on;
}
