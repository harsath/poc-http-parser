#!/usr/bin/php
<?php
// poc-http-parser
// Copyright © Harsath
// The software is licensed under the MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// Use: script.php (NUM_COMP)* output_file_name.h
// Ex: script.php 3 4 fast_string_cmp.h
function generate_macro($num) : string {
        $returner = "#define str".$num."cmp_macro(ptr, ";
        for($x = 0; $x < $num; $x++){
                $returner .= "c".$x;
                if($x != $num-1){ $returner .= ", "; }
        }
        $returner .= ") ";
        for($x = 0; $x < $num; $x++){
                $returner .= "*(ptr+".$x.") == c".$x;
                if($x != $num-1){ $returner .= " && "; }
        }
        return $returner;
}

function generate_static_inline_fn(&$generated_macro, $num) : string {
        $generated_macro .= "__attribute__((always_inline))\nstatic inline bool str".$num."cmp(const char* ptr, const char* cmp)".
                                "{\n\t\treturn str".$num."cmp_macro(ptr, ";
        for($x = 0; $x < $num; $x++){
                $generated_macro .= " *(cmp+".$x.")";
                if($x != $num-1){ $generated_macro .= ", "; }
        }
        $generated_macro .= ");\n}\n";
        return $generated_macro;
}

function handle_generation($argc, $argv) : void {
        $out_filename = $argv[$argc-1];
        $gen_macro = "";
        for($x = 0; $x < $argc-2; $x++){
                $macro = generate_macro($argv[$x+1])."\n";
                $gen_macro .= generate_static_inline_fn($macro, $argv[$x+1]);
        }
        file_put_contents($out_filename, $gen_macro);
}

handle_generation($argc, $argv);
?>
