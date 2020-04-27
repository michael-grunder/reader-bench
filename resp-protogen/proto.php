<?php

function randString($len) {
    $s = 0;

    while(strlen($s) < $len) {
        $s .= md5(uniqid());
    }

    return substr($s, 0, $len);
}

function randArray(&$arr, $leaf, $depth) {
    global $intmax;

    if (!$depth)  {
        switch ($leaf) {
            case 'int':
                $arr = rand(1, pow(2, 31));
                break;
            case 'bulk':
                $arr = randString(100);
                break;
            case 'multibulk':
                $sub = [];
                for ($i = 0; $i < rand(1, 4); $i++) {
                   $sub[] = rand(1, $intmax);
                }
                $arr = $sub;
                break;
            default:
                exit("???\n");
        }
        return;
    }

    $id = uniqid();
    $arr[$id] = NULL;
    randArray($arr[$id], $leaf, $depth - 1);
}

function respLine($byte, $data) {
    return $byte . $data . "\r\n";
}

function respInteger($min, $max) {
    return respLine(':', rand($min, $max));
}

function respStatus($minlen, $maxlen) {
    return respLine('+', randString($minlen, $maxlen));
}

function respBulk($minlen, $maxlen) {
    $str = randString(rand($minlen, $maxlen));
    return "$" . strlen($str) . "\r\n" . $str . "\r\n";
}

function respMultiBulk($depth, $leaf, $minlen, $maxlen, $intmax) {
    $resp = "";

    for ($i = 0; $i < $depth; $i++) {
        $resp .= "*1\r\n";
    }

    switch ($leaf) {
        case 'int':
            $resp .= respInteger(1, $intmax);
            break;
        case 'bulk':
            $resp .= respBulk($minlen, $maxlen);
            break;
        case 'multibulk':
            $n = rand(1, 4);
            $resp .= "*$n\r\n";
            for ($i = 0; $i < $n; $i++) {
                $resp .= respBulk($minlen, $maxlen);
            }
            break;
        default:
            exit("Unknown leaf type: $leaf\n");
    }

    return $resp;
}

$opt = getopt('', ['type:', 'maxdepth:', 'count:', 'intmax:', 'maxlen:', 'bulkleaf:', 'delim:']);

$count = $opt['count'] ?? 100;
$maxdepth = $opt['maxdepth'] ?? 3;
$type = $opt['type'] ?? 'bulk';
$intmax = $opt['intmax'] ?? pow(2, 31);
$maxlen = $opt['maxlen'] ?? 255;
$bulkleaf = $opt['bulkleaf'] ?? 'int';
$delim = $opt['delim'] ?? '';

$types = ['int', 'bulk', 'multibulk', 'status'];
if ($type != 'rand' && !in_array($type, $types)) {
    fprintf(STDERR, "Error:  Type must be on of " . implode(',', $types) . ', rand') . "\n";
    exit(-1);
}

$leaf_types = ['int', 'bulk', 'multibulk'];
$arr_replies = [];

$st = microtime(true);

for ($i = 0; $i < $count; $i++) {
    if ($type == 'rand') {
        $use_type = $types[array_rand($types)];
    } else {
        $use_type = $type;
    }

    switch ($use_type) {
        case 'int':
            $resp = respInteger(1, $intmax);
            break;
        case 'bulk':
            $resp = respBulk(1, $maxlen);
            break;
        case 'status':
            $resp = respStatus(1, $maxlen);
            break;
        case 'multibulk':
            if ($bulkleaf == 'rand') {
                $useleaf = $leaf_types[array_rand($leaf_types)];
            } else {
                $useleaf = $bulkleaf;
            }

            $resp = respMultiBulk(rand(1,$maxdepth), $useleaf, 1, $maxlen, $intmax);
            break;
        default:
            exit("Fatal:  Unknown type $use_type\n");
    }

    if ($i % 100 == 0) {
        $et = microtime(true);
        if ($et - $st >= 1) {
            $cur = $i + 1;
            $pct = round(100.00 * $cur/$count, 2);
            fprintf(STDERR, "[$cur/$count]: $pct\n");
            $st = $et;
        }
    }

    echo $resp;
    if ($delim) echo $delim;
}
