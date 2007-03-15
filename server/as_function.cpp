// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "as_function.h"
#include "builtin_function.h" // for _global.Function
#include "as_value.h"
#include "array.h"
#include "gnash.h"
#include "fn_call.h"
#include "GnashException.h"
#include "VM.h"

#include <typeinfo>
#include <iostream>

using namespace std;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

namespace gnash {

// should be static, probably
void function_apply(const fn_call& fn);
void function_call(const fn_call& fn);
static as_object* getFunctionPrototype();
static void function_ctor(const fn_call& fn);

/* 
 * This function returns the singleton
 * instance of the ActionScript Function object
 * prototype, which is what the AS Function class
 * exports, thus what each AS function instance inherit.
 *
 * The returned object can be accessed by ActionScript
 * code through Function.__proto__.prototype.
 * User AS code can add or modify members of this object
 * to modify behaviour of all Function AS instances.
 *
 * FIXME: do not use a static specifier for the proto
 * object, as multiple runs of a single movie should
 * each use a 'clean', unmodified, version of the
 * prototype. What should really happen is that this
 * prototype gets initializated by initialization of
 * the Function class itself, which would be a member
 * of the _global object for each movie instance.
 * 
 */
static as_object* getFunctionPrototype()
{
	// Make sure the prototype is always
	// alive (static boost::intrusive_ptr<> should ensure this)
	static boost::intrusive_ptr<as_object> proto;

	if ( proto.get() == NULL ) {
		// Initialize Function prototype
		proto = new as_object();

		if ( VM::get().getSWFVersion() >= 6 )
		{
			proto->init_member("apply", new builtin_function(function_apply));
			proto->init_member("call", new builtin_function(function_call));
		}
	}

	return proto.get();

}

static void
function_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> func = new as_object(getFunctionPrototype());
	//log_msg("User tried to invoke new Function()");
	if ( fn.result )
	{
		fn.result->set_as_object(func.get());
	}
}



// What if we want a function to inherit from Object instead ?
as_function::as_function(as_object* iface)
	:
	// all functions inherit from global Function class
	as_object(getFunctionPrototype()),
	_properties(iface)
{
	/// TODO: create properties lazily, on getPrototype() call
	if ( ! _properties )
	{
		_properties = new as_object();
	}
	_properties->init_member("constructor", this); 
	init_member("prototype", as_value(_properties.get()));
}

void
as_function::setPrototype(as_object* proto)
{
	_properties = proto;
	init_member("prototype", as_value(_properties.get()));
}

void
as_function::extends(as_function& superclass)
{
	_properties = new as_object(superclass.getPrototype());
	_properties->init_member("constructor", &superclass); 
	init_member("prototype", as_value(_properties.get()));
}

as_object*
as_function::getPrototype()
{
	// TODO: create if not available ?
	// TODO WARNING: what if user overwrites the 'prototype' member ?!
	//               this function should likely return the *new*
	//               prototype, not the old !!
	as_value proto;
	get_member("prototype", &proto);
	if ( proto.to_object() != _properties.get() )
	{
		log_warning("Exported interface of function %p has been overwritten (from %p to %p)!", this, _properties.get(), proto.to_object());
		_properties = proto.to_object();
	}
	return _properties.get();
}

/*
 * Initialize the "Function" member of a _global object.
 */
void
function_class_init(as_object& global)
{
	// This is going to be the global Function "class"/"function"
	// TODO: use Function() instead (where Function derives from as_function, being a class)

	// Make sure the prototype is always alive
	// (static boost::intrusive_ptr<> should ensure this)
	static boost::intrusive_ptr<as_function> func=new builtin_function(
		function_ctor, // function constructor doesn't do anything
		getFunctionPrototype() // exported interface
		);

	// Register _global.Function
	global.init_member("Function", func.get());

}

// Wrapper around dynamic_cast to implement user warning.
// To be used by builtin properties and methods.
static as_function*
ensureFunction(as_object* obj)
{
	as_function* ret = dynamic_cast<as_function*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for Function objects called against non-Function instance");
	}
	return ret;
}

void
function_apply(const fn_call& fn)
{
	int pushed=0; // new values we push on the stack

	// Get function body 
	as_function* function_obj = ensureFunction(fn.this_ptr);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);
	new_fn_call.nargs=0;

	if ( ! fn.nargs )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror ("Function.apply() called with no args");
		);
	}
	else
	{
		// Get the object to use as 'this' reference
		as_object *this_ptr = fn.arg(0).to_object();
		if ( this_ptr ) new_fn_call.this_ptr = this_ptr;
		// ... or recycle this function's call 'this' pointer
		// (most likely the Function instance)
		else new_fn_call.this_ptr = fn.this_ptr;

		if ( fn.nargs > 1 )
		// we have an 'arguments' array
		{
			IF_VERBOSE_ASCODING_ERRORS(
				if ( fn.nargs > 2 )
				{
					log_aserror("Function.apply() got %d"
						" args, expected at most 2"
						" -- discarding the ones in"
						" excess",
						fn.nargs);
				}
			);

			as_object *arg1 = fn.arg(1).to_object();
			if ( ! arg1 )
			{
				IF_VERBOSE_ASCODING_ERRORS(
					log_aserror("Second arg of Function.apply"
						" is of type %s, with value %s"
						" (expected array)"
						" - considering as call with no args",
						fn.arg(1).typeOf(),
						fn.arg(1).to_string());
				);
				goto call_it;
			}

			as_array_object *arg_array = \
					dynamic_cast<as_array_object*>(arg1);

			if ( ! arg_array )
			{
				IF_VERBOSE_ASCODING_ERRORS(
					log_aserror("Second arg of Function.apply"
						" is of type %s, with value %s"
						" (expected array)"
						" - considering as call with no args",
						fn.arg(1).typeOf(),
						fn.arg(1).to_string());
				);
				goto call_it;
			}

			unsigned int nelems = arg_array->size();

			//log_error("Function.apply(this_ref, array[%d])\n", nelems);
			as_value index, value;
			for (unsigned int i=nelems; i; i--)
			{
				value=arg_array->at(i-1);
				fn.env->push_val(value);
				pushed++;
			}

			new_fn_call.first_arg_bottom_index=fn.env->get_top_index();
			new_fn_call.nargs=nelems;
		}
	}

	call_it:

	// Call the function 
	(*function_obj)(new_fn_call);

	// Drop additional values we pushed on the stack 
	fn.env->drop(pushed);

}

void
function_call(const fn_call& fn)
{

	// Get function body 
	as_function* function_obj = ensureFunction(fn.this_ptr);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);

	if ( ! fn.nargs )
	{
                dbglogfile << "Function.call() with no args" << endl;
		new_fn_call.nargs=0;
	}
	else
	{
		// Get the object to use as 'this' reference
		as_object *this_ptr = fn.arg(0).to_object();
		new_fn_call.this_ptr = this_ptr;
		new_fn_call.nargs--;
		new_fn_call.first_arg_bottom_index--;
	}

	// Call the function 
	(*function_obj)(new_fn_call);

	//log_msg("at function_call exit, stack: \n"); fn.env->dump_stack();

	//log_msg("%s: tocheck \n", __FUNCTION__);
}

} // end of gnash namespace

