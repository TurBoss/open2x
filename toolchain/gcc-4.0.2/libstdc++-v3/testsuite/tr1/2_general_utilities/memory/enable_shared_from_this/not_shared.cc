// { dg-do run { xfail *-*-* } }
// Copyright (C) 2005 Free Software Foundation
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

// 2.2.5 Template class enable_shared_from_this [tr.util.smartptr.enab]

#include <tr1/memory>
#include <testsuite_hooks.h>

struct X : public std::tr1::enable_shared_from_this<X>
{
};

int
test01()
{
  bool test __attribute__((unused)) = true;

  X x;

  try
  {
    std::tr1::shared_ptr<X> p = x.shared_from_this();
  }
  catch (const std::tr1::bad_weak_ptr&)
  {
    // Expected.
    __throw_exception_again;
  }
  catch (...)
  {
    // Failed.
  }

  return 0;
}

int 
main()
{
  test01();
  return 0;
}
