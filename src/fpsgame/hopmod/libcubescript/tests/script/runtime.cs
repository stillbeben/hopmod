
echo $FILENAME

alias assert [if (! (arg1)) [throw runtime.assert_failure]] // Setup test script

assert [symbol? throw]

assert [symbol? result]
assert [0; 1]
assert [result 1; 0]

assert [symbol? push]
assert [ i = 0; push i 1; $i]

assert [symbol? pop]
assert [ i = 1; push i 0; pop i; $i]

// Number comparison functions

assert [symbol? =]

assert [symbol? >]
assert [> 10 5]

assert [symbol? >=]
assert [>= 10 5]
assert [>= 10 10]

assert [symbol? <]
assert [< 5 10]

assert [symbol? <=]
assert [<= 5 10]
assert [<= 5 5]

// Arthmetic functions

assert [symbol? +]
assert [= (+ 1 2) 3]
assert [= (+ 1.5 2) 3]
assert [= (+ -2 -3) -5]

assert [symbol? -]
assert [= (- 10 5) 5]
assert [= (- 10.8 5) 5]
assert [= (- -10 -10) 0]

assert [symbol? *]
assert [= (* 2 3) 6]
assert [= (* 2.7 3) 6]
assert [= (* -2 -2) 4]

assert [symbol? div]
assert [= (div 10 5) 2]
assert [= (div 10 3) 3]
assert [= (div 10 -2) -5]

assert [symbol? mod]
assert [= (mod 6 5) 1]
// the mod function behaves the same as the C++ % operator

assert [symbol? min]
assert [= (min 3 8) 3]
assert [= (min 8 3) 3]

assert [symbol? max]
assert [= (max 3 8) 8]
assert [= (max 8 3) 8]

// Boolean logic functions

assert [symbol? !]
assert [! 0]
assert [= (! 1) 0]

assert [symbol? ||]
assert [|| 1 0]
assert [|| 2 2]
assert [|| 0 3]
assert [! (|| 0 0)]

assert [symbol? &&]
assert [&& 1 1]
assert [= (&& 1 0) 0]

assert [symbol? ^]
assert [= (^ 1 1) 0]
assert [= (^ 1 0) 1]
assert [= (^ 0 0) 0]

// Control flow functions

assert [symbol? if]
assert [i = -1; if 1 [i = 1] [i = 0]; = $i 1]
assert [i = -1; if 0 [i = 1] [i = 0]; = $i 0]

assert [symbol? loop]
i = 5 //some initial value
tracker = -1
loop i 10 [
    assert [= $i (+ $tracker 1)]
    tracker = (+ $tracker 1)
]
assert [= $i 5] //check to see the same initial value is set

assert [symbol? while]
i = 5
while [> $i 0] [i = (- $i 1)]
assert [! $i]

// String and list functions

assert [symbol? strcmp]
assert [strcmp hello hello]
assert [! (strcmp hello bye)]

assert [symbol? at]
assert [strcmp (at [first second "third element"] 2) "third element"]

assert [symbol? listlen]
assert [= (listlen ["first element" 
                    second 
                    "third element" forth "fifth element"]) 5]

assert [symbol? concat]
assert [strcmp (concat "one two three" four five) "one two three four five"]

assert [symbol? concatword]
assert [strcmp (concatword pres ident) president]

assert [symbol? format]
assert [strcmp (format "movement for %1" "democratic change") "movement for democratic change"]
