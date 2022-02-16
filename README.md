LITMUS-RT liblitmus Python Module
=================================

This module exposes `liblitmus` (used to interact with
[LITMUS^RT](litmus-rt.org)) in python scripts. It includes some behavior
specific to my own research, including a modified version of LITMUS. The
respository containing my version of LITMUS can be found
[here](https://github.com/yalue/litmus-rt/tree/add_kfmlp) (note the non-default
branch).

Installation
============

Note that this library requires python 3, my modified version of LITMUS^RT, and
my [modified version of liblitmus](github.com/yalue/liblitmus). To install this
library, clone the project, `cd` into the directory, and modify `setup.py` so
that the paths to your `liblitmus` installation is accurate. (You will need to
compile `liblitmus` ahead of time.) Next, run `python setup.py install`.

Usage
=====

After installing the library, you can use it in python code as follows:

```python
import liblitmus_helper as liblitmus

# View the available functions and documentation
help(liblitmus)
```

