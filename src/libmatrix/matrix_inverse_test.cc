//
// Copyright (c) 2010 Linaro Limited
//
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the MIT License which accompanies
// this distribution, and is available at
// http://www.opensource.org/licenses/mit-license.php
//
// Contributors:
//     Jesse Barker - original implementation.
//
#include <iostream>
#include "mat.h"

using LibMatrix::mat4;
using LibMatrix::mat3;
using LibMatrix::mat2;
using std::cerr;
using std::cout;
using std::endl;

bool mat2OK()
{
    mat2 m;
    cout << "Starting with mat2 (should be identity): " << endl << endl;
    m.print();

    m[0][1] = -2.5;
    
    cout << endl << "Matrix should now have (0, 1) == -2.500000" << endl << endl;
    m.print();
    
    mat2 mi(m);

    cout << endl << "Copy of previous matrix (should have (0, 1) == -2.500000)" << endl << endl;
    mi.print();

    mi.inverse();

    cout << endl << "Inverse of copy: " << endl << endl;
    mi.print();

    mat2 i = m * mi;

    cout << endl << "Product of original and inverse (should be identity): " << endl << endl;
    i.print();

    mat2 ident;
    if (i != ident)
    {
        return false;
    }

    return true;
}

bool mat3OK()
{
    mat3 m;
    cout << "Starting with mat3 (should be identity): " << endl << endl;
    m.print();

    m[1][2] = -2.5;
    
    cout << endl << "Matrix should now have (1, 2) == -2.500000" << endl << endl;
    m.print();
    
    mat3 mi(m);

    cout << endl << "Copy of previous matrix (should have (1, 2) == -2.500000)" << endl << endl;
    mi.print();

    mi.inverse();

    cout << endl << "Inverse of copy: " << endl << endl;
    mi.print();

    mat3 i = m * mi;

    cout << endl << "Product of original and inverse (should be identity): " << endl << endl;
    i.print();

    mat3 ident;
    if (i != ident)
    {
        return false;
    }

    return true;
}

bool mat4OK()
{
    mat4 m;
    cout << "Starting with mat4 (should be identity): " << endl << endl;
    m.print();

    m[2][3] = -2.5;
    
    cout << endl << "Matrix should now have (2, 3) == -2.500000" << endl << endl;
    m.print();
    
    mat4 mi(m);

    cout << endl << "Copy of previous matrix (should have (2, 3) == -2.500000)" << endl << endl;
    mi.print();

    mi.inverse();

    cout << endl << "Inverse of copy: " << endl << endl;
    mi.print();

    mat4 i = m * mi;

    cout << endl <<  "Product of original and inverse (should be identity): " << endl << endl;
    i.print();

    mat4 ident;
    if (i != ident)
    {
        return false;
    }

    return true;
}

int
main(int argc, char** argv)
{
    if (!mat2OK())
    {
        cerr << "mat2::inverse() does not work!" << endl;
        return 1;
    }
    cout << "mat2::inverse() is okay!" << endl << endl;

    if (!mat3OK())
    {
        cerr << "mat3::inverse() does not work!" << endl;
        return 1;
    }
    cout << "mat3::inverse() is okay!" << endl << endl;

    if (!mat4OK())
    {
        cerr << "mat4::inverse() does not work!" << endl;
        return 1;
    }
    cout << "mat4::inverse() is okay!" << endl << endl;

    return 0;
}
