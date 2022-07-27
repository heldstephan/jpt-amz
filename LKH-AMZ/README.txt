LKH code for the Last Mile Routing Research Challenge.

INSTALLATION
------------

Download the software and execute the following UNIX commands:

    tar xvfz LKH-AMZ.tgz
    cd LKH-AMZ
    make

Two executable files called LKH and score will now be available
in the directory LKH-AMZ.

The script solve_and_merge is used for finding LKH tours for instances
in two given directories and compute the score of the merged tours.
You may want to change the number of THREADS in the script solve from
its default value of 16.

    Example of use:

        ./solve_and_merge Path Pred Merged

            or

       ./solve Path
       ./solve Pred
       ./merge Path Pred Merged

LICENSE

MIT License

Copyright (c) 2021 Keld Helsgaun

Permission is hereby granted, free of charge, 
to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to 
deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, 
merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom 
the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice 
shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
