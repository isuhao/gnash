// Object.cpp:  Implementation of ActionScript Object class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//


#include "tu_config.h"
#include "Object.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "movie_definition.h" // for Object.registerClass (get_exported_resource)
//#include "character.h" // for Object.registerClass  (get_root_movie)
#include "sprite_instance.h" // for Object.registerClass  (get_movie_definition)
#include "sprite_definition.h" // for Object.registerClass  (get_movie_definition)
#include "VM.h" // for SWF version (attachObjectInterface)

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {

// Forward declarations
static as_value object_addproperty(const fn_call&);
static as_value object_registerClass(const fn_call& fn);
static as_value object_hasOwnProperty(const fn_call&);
static as_value object_isPropertyEnumerable(const fn_call&);
static as_value object_isPrototypeOf(const fn_call&);
static as_value object_watch(const fn_call&);
static as_value object_unwatch(const fn_call&);


static void
attachObjectInterface(as_object& o)
{
	VM& vm = o.getVM();
	int target_version = vm.getSWFVersion();

	// Object.valueOf()
	vm.registerNative(as_object::valueof_method, 101, 3);
	o.init_member("valueOf", vm.getNative(101, 3));

	// Object.toString()
	vm.registerNative(as_object::tostring_method, 101, 4);
	o.init_member("toString", vm.getNative(101, 4));

	if ( target_version  < 6 ) return;

	// Object.addProperty()
	vm.registerNative(object_addproperty, 101, 2);
	o.init_member("addProperty", vm.getNative(101, 2));

	// Object.hasOwnProperty()
	vm.registerNative(object_hasOwnProperty, 101, 5);
	o.init_member("hasOwnProperty", vm.getNative(101, 5));

	// Object.isPropertyEnumerable()
	vm.registerNative(object_isPropertyEnumerable, 101, 7);
	o.init_member("isPropertyEnumerable", vm.getNative(101, 7));

	// Object.isPrototypeOf()
	vm.registerNative(object_isPrototypeOf, 101, 6);
	o.init_member("isPrototypeOf", vm.getNative(101, 6));

	// Object.watch()
	vm.registerNative(object_watch, 101, 0);
	o.init_member("watch", vm.getNative(101, 0));

	// Object.unwatch()
	vm.registerNative(object_unwatch, 101, 1);
	o.init_member("unwatch", vm.getNative(101, 1));
}

as_object*
getObjectInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object(); // end of the inheritance chain
		attachObjectInterface(*o);
		//o->set_prototype(o.get()); // proto is self ?
	}
	return o.get();
}

// FIXME: add some useful methods :)
class object_as_object : public as_object
{

public:

	object_as_object()
		:
		as_object(getObjectInterface())
	{
	}

};

static as_value
object_ctor(const fn_call& fn)
    // Constructor for ActionScript class Object.
{
	if ( fn.nargs == 1 ) // copy constructor
	{
		// just copy the reference
		//
		// WARNING: it is likely that fn.result and fn.arg(0)
		// are the same location... so we might skip
		// the set_as_object() call as a whole.
		return as_value(fn.arg(0).to_object());
	}

	boost::intrusive_ptr<as_object> new_obj;
	if ( fn.nargs == 0 )
	{
		new_obj = new object_as_object();
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS (
		log_aserror(_("Too many args to Object constructor"));
		)
		new_obj = new object_as_object();
	}

	return as_value(new_obj.get()); // will keep alive
}

std::auto_ptr<as_object>
init_object_instance()
{
	return std::auto_ptr<as_object>(new object_as_object);
}


// extern (used by Global.cpp)
void object_class_init(as_object& global)
{
	// This is going to be the global Object "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl=NULL;

	VM& vm = VM::get();

	if ( cl == NULL )
	{
		cl=new builtin_function(&object_ctor, getObjectInterface());

		// TODO: is this needed ?
		//cl->init_member("prototype", as_value(getObjectInterface()));

		// Object.registerClass() -- TODO: should this only be in SWF6 or higher ?
		vm.registerNative(object_registerClass, 101, 8);
		cl->init_member("registerClass", vm.getNative(101, 8));

		     
	}

	// Register _global.Object
	global.init_member("Object", cl.get());

}

static as_value
object_addproperty(const fn_call& fn)
{
	assert(fn.this_ptr);
	boost::intrusive_ptr<as_object> obj = fn.this_ptr;

	if ( fn.nargs != 3 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.addProperty(%s) - "
			"expected 3 arguments (<name>, <getter>, <setter>)"),
		       	ss.str().c_str());
		);

		// if we've been given more args then needed there's
		// no need to abort here
		if ( fn.nargs < 3 )
		{
			return as_value(false);
		}
	}

	const std::string& propname = fn.arg(0).to_string();
	if ( propname.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.addProperty() - "
			"empty property name"));
		);
		return as_value(false);
	}

	as_function* getter = fn.arg(1).to_as_function();
	if ( ! getter )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.addProperty() - "
			"getter is not an AS function"));
		);
		return as_value(false);
	}

	as_function* setter = fn.arg(2).to_as_function();
	if ( ! setter )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.addProperty() - "
			"setter is not an AS function"));
		);
		return as_value(false);
	}


	// Now that we checked everything, let's call the as_object
	// interface for getter/setter properties :)
	
	bool result = obj->add_property(propname, *getter, *setter);

	//log_debug("Object.addProperty(): testing");
	return as_value(result);
}

static as_value
object_registerClass(const fn_call& fn)
{
	assert(fn.this_ptr);
	//as_object* obj = fn.this_ptr;

	if ( fn.nargs != 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.registerClass(%s) - "
			"expected 2 arguments (<symbol>, <constructor>)"),
			ss.str().c_str());
		);

		// if we've been given more args then needed there's
		// no need to abort here
		if ( fn.nargs < 2 )
		{
			return as_value(false);
		}
	}

	const std::string& symbolid = fn.arg(0).to_string();
	if ( symbolid.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.registerClass(%s) - "
			"first argument (symbol id) evaluates to empty string"), ss.str().c_str());
		);
		return as_value(false);
	}

	as_function* theclass = fn.arg(1).to_as_function();
	if ( ! theclass )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		std::stringstream ss;
		fn.dump_args(ss);
		log_aserror(_("Invalid call to Object.registerClass(%s) - "
			"second argument (class) is not a function)"), ss.str().c_str());
		);
		return as_value(false);
	}

	// Find the exported resource

	// TODO: check to *which* definition should we ask the export
	//       this code uses the *relative* root of current environment's target
	movie_definition* def = VM::get().getRoot().get_movie_definition();
	boost::intrusive_ptr<resource> exp_res = def->get_exported_resource(symbolid.c_str());
	if ( ! exp_res )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.registerClass(%s, %s): "
			"can't find exported symbol"),
			symbolid.c_str(), 
			typeid(theclass).name());
		);
		return as_value(false);
	}

	// Check that the exported resource is a sprite_definition
	// (we're looking for a MovieClip symbol)

	boost::intrusive_ptr<sprite_definition> exp_clipdef = 
		boost::intrusive_ptr<sprite_definition>(dynamic_cast<sprite_definition*>(exp_res.get()));


	if ( ! exp_clipdef )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.registerClass(%s, %s): "
			"exported symbol is not a MovieClip symbol "
			"(sprite_definition), but a %s"),
			symbolid.c_str(), 
			typeid(theclass).name(),
			typeid(*exp_res).name());
		);
		return as_value(false);
	}

	exp_clipdef->registerClass(theclass);
	return as_value(true);
}

as_value
object_hasOwnProperty(const fn_call& fn)
{
	//assert(fn.result->is_undefined());
	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.hasOwnProperty() requires one arg"));
		);
		return as_value(false);
	}
	as_value& arg = fn.arg(0);
	const std::string& propname = arg.to_string();
	if ( arg.is_undefined() || propname.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.hasOwnProperty('%s')"), arg.to_debug_string().c_str());
		);
		return as_value(false);
	}
	log_debug("%p.hasOwnProperty", fn.this_ptr);
	return as_value(fn.this_ptr->getOwnProperty(VM::get().getStringTable().find(propname)) != NULL);
}

as_value
object_isPropertyEnumerable(const fn_call& fn)
{
	//assert(fn.result->is_undefined());
	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.isPropertyEnumerable() requires one arg"));
		);
		return as_value();
	}
	as_value& arg = fn.arg(0);
	const std::string& propname = arg.to_string();
	if ( arg.is_undefined() || propname.empty() )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Invalid call to Object.isPropertyEnumerable('%s')"), arg.to_debug_string().c_str());
		);
		return as_value();
	}

	Property* prop = fn.this_ptr->getOwnProperty(VM::get().getStringTable().find(propname));
	if ( ! prop )
	{
		return as_value(false);
	}

	return as_value( ! prop->getFlags().get_dont_enum() );
}

as_value
object_isPrototypeOf(const fn_call& fn)
{
	//assert(fn.result->is_undefined());
	if ( fn.nargs < 1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("Object.isPrototypeOf() requires one arg"));
		);
		return as_value(false); 
	}

	boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
	if ( ! obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror(_("First arg to Object.isPrototypeOf(%s) is not an object"), fn.arg(0).to_debug_string().c_str());
		);
		return as_value(false);
	}

	return as_value(fn.this_ptr->prototypeOf(*obj));

}

as_value
object_watch(const fn_call&)
{
	static bool warned = false;
	if ( ! warned ) {
		log_unimpl (__FUNCTION__);
		warned=true;
	}
	return as_value();
}

as_value
object_unwatch(const fn_call&)
{
	static bool warned = false;
	if ( ! warned ) {
		log_unimpl (__FUNCTION__);
		warned=true;
	}
	return as_value();
}
  
} // namespace gnash
