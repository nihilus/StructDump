Intro
========================

StructDump is an IDA plugin, allowing you to export IDA types (structs and enums) into
high-level language definitions.
In addition to that, the "Make table" functionality will allow you to create arrays from bytes inside the database.

C++/Delphi is supported; adding support for other languages is simple.

The source code is released under the GNU GPL.
If you modify/find bugs/enhance this code, please send me the changes.

Usage
========================

The usage of the plugin is pretty simple

You can add as many structs as you want, without worrying about structs interelation, since all linked structs will be automatically
retrieved.

After you select structs, you may choose to export to file or you may clear the list of exported structures.


Configuration
========================

StDump uses Stdump.ini file to read its configuration data.

You will find the following values:

Types mapping
---------------
dt_xxxxxx=yyyyyyyy

example:
dt_byte=char

This means to map IDA's byte type to C's "char"

Comment Styles
-----------------

The "cmt_style" key allows you to control the format of the output comment

  ECS_DOXY = 1 // emit comment style = doxygen
  ECS_NORMAL = 2 // normal comments
  ECS_NONE = 0 // no comments


The "cmt_sizeof" will display the sizes of each struct in the output as comments

The "cmt_showoffs" will display the offsets of each struct member

Strings
---------

Example: 
  union=union
  struct=struct
  enum=enum

The purpose of this key is to hold the keywords used in a certain language output.
If we change these keywords and modified the emit function, we can output to different languages

Pointers
-----------

The "void_ptr" key allows you to set the default data type of a pointer,
Example, if you have a "long pTest ; offset" definition in IDA and this key:
void_ptr=void

then the output will be: "void *pTest"
To turn this option of, just clear this key and the output will be: "long *pTest"

Comment markup
----------------
The non permanent struct member comment can be used to store some special codes.

The "ptr_to":
~~~~~~~~~~~~~~
Example:
pMyStruct1 dd ? ; offset
If you add a comment to this member as: "your definition.....(ptr_to:MYSTRUCT)......"
Then the output will be:
MYSTRUCT *pMyStruct1


Other
--------
The "ptr_size" is used to specify the pointer size.
It is important to set a correct value, for StDump will consider "pointer" only members that are ptr_size bytes long
and have "offset" flag.


License
========================

/* 
* StDump
*
*	Copyright (C) lallous <lallousx86@yahoo.com>
*
*  StructDump is an IDA plugin, allowing you to export IDA types into
*  high-level language definitions.
*	
*
*  StructDump is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*   
*  StructDump is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*   
*  You should have received a copy of the GNU General Public License
*  along with GNU Make; see the file COPYING.  If not, write to
*  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
*
*/
