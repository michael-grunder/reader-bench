<?php

$opt = getopt('dn:', ['path:', 'min-depth:', 'max-depth:', 'min-len:', 'max-len:', 'type:']);
$dtm = isset($opt['d']);

$type = $opt['type'] ?? 'multibulk';
$n = $opt['n'] ?? 10;
$path = $opt['path'] ?? '/tmp/';
$mindepth = $opt['min-depth'] ?? 4;
$maxdepth = $opt['max-depth'] ?? 24;
$minlen = $opt['min-len'] ?? 10000;
$maxlen = $opt['max-len'] ?? 100000;

if ($maxdepth < $mindepth) $maxdepth = $mindepth;
if ($maxlen < $minlen) $maxlen = $minlen;

$obj_r = new Redis();
$obj_r->connect("127.0.0.1", 6379);
$obj_r->del("proto-files");

$on = 0;
while (true) {
    ++$on;

    if ($obj_r->llen("proto-files") >= $n) {
        echo "[$on]: Ten job backlog, waiting for some to finish: ";
        while ($obj_r->llen("proto-files") >= $n) {
            sleep(1);
        }
        echo "Continuing\n";
    }

    $count = rand($minlen, $maxlen);
    if (!$dtm) {
        $str_file = tempnam($path, "$type-$count-") . ".proto";
    } else {
        $str_file = "$path/$type-$count.$on.proto";
    }

    $depth = rand($mindepth, $maxdepth);
    $str_cmd = "php proto.php --type $type --count $count --maxdepth $depth --bulkleaf rand 2>/dev/null >| $str_file";

    exec($str_cmd, $arr_out, $exitcode);
    if ($exitcode != 0) {
        fprintf(STDERR, "Error:  Nonzero exit code from proto.php!\n");
        exit(-1);
    }

    echo "[$on]: Sending job '$str_file' (len: $count, max depth: $depth)\n";
    $obj_r->rpush("proto-files", $str_file);
    $obj_r->incr("proto-jobs");
}
