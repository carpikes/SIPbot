#!/bin/php
<?php
    $stdin = fopen('php://stdin', 'r');

    while($l = fgets($stdin)) {
        // Remove new line symbol
        if($l[strlen($l)-1] == "\n")
            $l = substr($l, 0, strlen($l)-1);

        // Split as <cmd> <args>
        $e = explode(" ", $l, 2);

        $arg = "";
        if(count($e) == 2)
            $arg = $e[1];

        switch($e[0]) {
            case "CALL":
                if($arg == '"YOUR_FAVORITE_NUMBER"')
                    echo "PLAY /tmp/file1.wav\n";
                if($arg == '"Anonymous"')
                    echo "PLAY /tmp/file2.wav\n";
                break;
            case "DTMF":
                switch($arg) {
                    case "5":
                        echo "PLAY /tmp/file1.wav\n";
                        break;
                    case "6":
                        echo "PLAY /tmp/file2.wav\n";
                        break;
                    case "9":
                        echo "STOP\n";
                        break;
                    case "0":
                        echo "KILL\n";
                        break;
                }
                break;

            default:
        }
    }
?>
