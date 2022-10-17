# Virtual Machine in C++
### This is a simple stack-based VM that can read in the S-Expression and complie them into bytecode, and finally execute them by interpreter. It has scope analysis, garbage collector. It also support lambda function and OOB programming.

## here are some example, you can skip to bottom
```
 ./VM -e "(begin (var p (+ 5 3) ) p)"

-------------Disassembly: main ---------------------

0000   01 00       CONST                0 (5)
0002   01 01       CONST                1 (3)
0004   02          ADD
0005   12 00       GET_LOCAL            0 (p)
0007   14 01       SCOPE_EXIT           1
0009   00          HALT

result = EvaValue(NUMBER): 8
```
#### code: defining normal function using global function
```

(begin 
	(var x (* 5 3))
	(def adder(y)
		(+ x y)
	)
	(var p (adder 6))
	p
)

```
```
./VM -f ..path to code

-------------Disassembly: main ---------------------

0000   01 00       CONST                0 (5)
0002   01 01       CONST                1 (3)
0004   04          MUL
0005   18 00       SET_CELL             0 (x)
0007   11          POP
0008   19 00       LOAD_CELL            0 (x)
000A   01 02       CONST                2 (code 0x557819bb75f0: adder/1)
000C   20 01       MAKE_FUNCTION        1
000E   12 00       GET_LOCAL            0 (adder)
0010   01 03       CONST                3 (6)
0012   15 01       CALL                 1
0014   12 01       GET_LOCAL            1 (p)
0016   14 02       SCOPE_EXIT           2
0018   00          HALT


-------------Disassembly: adder ---------------------

0000   17 00       GET_CELL             0 (x)
0002   12 01       GET_LOCAL            1 (y)
0004   02          ADD
0005   14 02       SCOPE_EXIT           2
0007   16          RETURN

result = EvaValue(NUMBER): 21
```

#### code: Using lambda fuction
```
(begin 
  (var square (lambda (x) (* x x))) 
  (square 2)
)
```
```
./VM -f ..path to code

-------------Disassembly: main ---------------------

0000   01 01       CONST                1 (square/1)
0002   12 00       GET_LOCAL            0 (square)
0004   01 02       CONST                2 (2)
0006   15 01       CALL                 1
0008   14 01       SCOPE_EXIT           1
000A   00          HALT


-------------Disassembly: square ---------------------

0000   12 01       GET_LOCAL            1 (x)
0002   12 01       GET_LOCAL            1 (x)
0004   04          MUL
0005   14 02       SCOPE_EXIT           2
0007   16          RETURN

result = EvaValue(NUMBER): 4
```

#### OOB programming!
```
(class Point null
    (def constructor (self x y)
        (begin
            (set (prop self x) x)
            (set (prop self y) y)
                self
            )
    )

    (def calc (self)
        (+ (prop self x) (prop self y))
    )
)

(class Point3D Point
    (def constructor (self x y z)
        (begin
            ((prop (super Point3D) constructor) self x y)
                (set (prop self z) z)
                self
            )
        )

    (def calc (self)

        (+ ((prop (super Point3D) calc) self) (prop self z))
    )
)

(var p (new Point3D 10 20 30))

((prop p calc) p) //pass p as self
```

```
-------------Disassembly: main ---------------------

0000   09 04       GET_GLOBAL           4 (Point3D)
0002   21          NEW
0003   01 06       CONST                6 (10)
0005   01 07       CONST                7 (20)
0007   01 08       CONST                8 (30)
0009   15 04       CALL                 4
000B   10 05       SET_GLOBAL           5 (p)
000D   09 05       GET_GLOBAL           5 (p)
000F   22 09       GET_PROP             9 (calc)
0011   09 05       GET_GLOBAL           5 (p)
0013   15 01       CALL                 1
0015   00          HALT


-------------Disassembly: Point.constructor ---------------------

0000   12 02       GET_LOCAL            2 (x)
0002   12 01       GET_LOCAL            1 (self)
0004   23 00       SET_PROP             0 (x)
0006   11          POP
0007   12 03       GET_LOCAL            3 (y)
0009   12 01       GET_LOCAL            1 (self)
000B   23 01       SET_PROP             1 (y)
000D   11          POP
000E   12 01       GET_LOCAL            1 (self)
0010   14 04       SCOPE_EXIT           4
0012   16          RETURN


-------------Disassembly: Point.calc ---------------------

0000   12 01       GET_LOCAL            1 (self)
0002   22 00       GET_PROP             0 (x)
0004   12 01       GET_LOCAL            1 (self)
0006   22 01       GET_PROP             1 (y)
0008   02          ADD
0009   14 02       SCOPE_EXIT           2
000B   16          RETURN


-------------Disassembly: Point3D.constructor ---------------------

0000   09 03       GET_GLOBAL           3 (Point)
0002   22 00       GET_PROP             0 (constructor)
0004   12 01       GET_LOCAL            1 (self)
0006   12 02       GET_LOCAL            2 (x)
0008   12 03       GET_LOCAL            3 (y)
000A   15 03       CALL                 3
000C   11          POP
000D   12 04       GET_LOCAL            4 (z)
000F   12 01       GET_LOCAL            1 (self)
0011   23 01       SET_PROP             1 (z)
0013   11          POP
0014   12 01       GET_LOCAL            1 (self)
0016   14 05       SCOPE_EXIT           5
0018   16          RETURN


-------------Disassembly: Point3D.calc ---------------------

0000   09 03       GET_GLOBAL           3 (Point)
0002   22 00       GET_PROP             0 (calc)
0004   12 01       GET_LOCAL            1 (self)
0006   15 01       CALL                 1
0008   12 01       GET_LOCAL            1 (self)
000A   22 01       GET_PROP             1 (z)
000C   02          ADD
000D   14 02       SCOPE_EXIT           2
000F   16          RETURN

result = EvaValue(NUMBER): 60
```
## Try it yourself
### requirements:
- Linux
- CMAKE
- C++ tools
### Download the code and go to the code directory
### then make a build folder
```
mkdir build
```

### then configure the CMake
```
cmake ../src
```

### build it !
```
make
```

### if things go well, you should able to use it
```
./VM
```
### Run the command above to see the instruction