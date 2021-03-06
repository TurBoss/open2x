// Copyright (C) 2003 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// 27.8.1.4 Overridden virtual functions

#include <iostream>
#include <locale>
#include <testsuite_hooks.h>

// libstdc++/13171
void test01()
{
  bool test __attribute__((unused)) = true;
  using namespace std;

  locale::global(__gnu_test::try_named_locale("fr_FR"));

  ios_base::sync_with_stdio(false);

  locale::global(__gnu_test::try_named_locale("en_US"));
  cin.imbue(__gnu_test::try_named_locale("en_US"));
  cout.imbue(__gnu_test::try_named_locale("en_US"));
  cerr.imbue(__gnu_test::try_named_locale("en_US"));
  clog.imbue(__gnu_test::try_named_locale("de_DE"));
  wcin.imbue(__gnu_test::try_named_locale("en_US"));
  wcout.imbue(__gnu_test::try_named_locale("en_US"));
  wcerr.imbue(__gnu_test::try_named_locale("en_US"));
  wclog.imbue(__gnu_test::try_named_locale("de_DE"));
 
  cout << 'f' << endl;
  cerr << 'r' << endl;
  clog << 'A' << endl;
  wcout << L's' << endl;
  wcerr << L'i' << endl;
  wclog << L'L' << endl;
}

int main()
{
  test01();
}
