// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

// Implementation and helpers for SWF actions.


#ifndef GNASH_ACTION_H
#define GNASH_ACTION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "gnash.h"
#include "as_object.h"
#include "types.h"
#include <wchar.h>

#include "container.h"
#include "smart_ptr.h"
//#include "Function.h"
#include "log.h"

namespace gnash {
	struct movie;
	struct as_environment;
	class as_object;
	struct as_value;
	struct function_as_object;


	extern smart_ptr<as_object> s_global;

	//
	// event_id
	//

	/// For keyDown and stuff like that.
	struct event_id
	{

		/// These must match the function names in event_id::get_function_name()
		enum id_code
		{
			INVALID,

			// These are for buttons & sprites.
			PRESS,
			RELEASE,
			RELEASE_OUTSIDE,
			ROLL_OVER,
			ROLL_OUT,
			DRAG_OVER,
			DRAG_OUT,
			KEY_PRESS,

			// These are for sprites only.
			INITIALIZE,
			LOAD,
			UNLOAD,
			ENTER_FRAME,
			MOUSE_DOWN,
			MOUSE_UP,
			MOUSE_MOVE,
			KEY_DOWN,
			KEY_UP,
			DATA,
			
                        // These are for the MoveClipLoader ActionScript only
                        LOAD_START,
                        LOAD_ERROR,
                        LOAD_PROGRESS,
                        LOAD_INIT,
			
                        // These are for the XMLSocket ActionScript only
                        SOCK_CLOSE,
                        SOCK_CONNECT,
                        SOCK_DATA,
                        SOCK_XML,
			
                        // These are for the XML ActionScript only
                        XML_LOAD,
                        XML_DATA,
			
                        // This is for setInterval
                        TIMER,
			
			EVENT_COUNT
		};

		unsigned char	m_id;
		unsigned char	m_key_code;

		event_id() : m_id(INVALID), m_key_code(key::INVALID) {}

		event_id(id_code id, key::code c = key::INVALID)
			:
			m_id((unsigned char) id),
			m_key_code((unsigned char) c)
		{
			// For the button key events, you must supply a keycode.
			// Otherwise, don't.
			assert((m_key_code == key::INVALID && (m_id != KEY_PRESS))
				|| (m_key_code != key::INVALID && (m_id == KEY_PRESS)));
		}

		bool	operator==(const event_id& id) const { return m_id == id.m_id && m_key_code == id.m_key_code; }

		/// Return the name of a method-handler function
		/// corresponding to this event.
		const tu_string&	get_function_name() const;
	};

	//
	// with_stack_entry
	//

	/// The "with" stack is for Pascal-like with-scoping.
	struct with_stack_entry
	{
		smart_ptr<as_object>	m_object;
		int	m_block_end_pc;
		
		with_stack_entry()
			:
			m_object(NULL),
			m_block_end_pc(0)
		{
		}

		with_stack_entry(as_object* obj, int end)
			:
			m_object(obj),
			m_block_end_pc(end)
		{
		}
	};


	/// Base class for actions.
	struct action_buffer
	{
		action_buffer();

		/// Read action bytes from input stream
		void	read(stream* in);

		/// \brief
		/// Interpret the actions in this action buffer, and evaluate
		/// them in the given environment. 
		//
		/// Execute our whole buffer,
		/// without any arguments passed in.
		///
		void	execute(as_environment* env);

		/// Interpret the specified subset of the actions in our buffer.
		//
		/// Caller is responsible for cleaning up our local
		/// stack frame (it may have passed its arguments in via the
		/// local stack frame).
		void	execute(
			as_environment* env,
			int start_pc,
			int exec_bytes,
			as_value* retval,
			const std::vector<with_stack_entry>& initial_with_stack,
			bool is_function2);

		bool	is_null()
		{
			return m_buffer.size() < 1 || m_buffer[0] == 0;
		}

		int	get_length() const { return m_buffer.size(); }

	private:
		// Don't put these as values in std::vector<>!  They contain
		// internal pointers and cannot be moved or copied.
		// If you need to keep an array of them, keep pointers
		// to new'd instances.
		action_buffer(const action_buffer& a) { assert(0); }
		void operator=(const action_buffer& a) { assert(0); }

		void	process_decl_dict(int start_pc, int stop_pc);

		// data:
		std::vector<unsigned char>	m_buffer;
		std::vector<const char*>	m_dictionary;
		int	m_decl_dict_processed_at;

		void doActionNew(as_environment* env, 
			std::vector<with_stack_entry>& with_stack);

		void doActionInstanceOf(as_environment* env);

		void doActionCast(as_environment* env);

		void doActionCallMethod(as_environment* env);

		void doActionCallFunction(as_environment* env,
			std::vector<with_stack_entry>& with_stack);

		void doActionDefineFunction(as_environment* env,
			std::vector<with_stack_entry>& with_stack, int pc, int* next_pc);

		void doActionDefineFunction2(as_environment* env,
			std::vector<with_stack_entry>& with_stack, int pc, int* next_pc);

		void doActionGetMember(as_environment* env);

		void doActionStrictEquals(as_environment* env);

		void doActionEquals(as_environment* env);

		void doActionDelete(as_environment* env,
			std::vector<with_stack_entry>& with_stack);

		void doActionDelete2(as_environment* env,
			std::vector<with_stack_entry>& with_stack);

	};


	struct fn_call;
	typedef void (*as_c_function_ptr)(const fn_call& fn);


	struct as_property_interface
	{
		virtual ~as_property_interface() {}
		virtual bool	set_property(int index, const as_value& val) = 0;
	};



// tulrich: I'm not too sure this is useful.  For things like
// xml_as_object, is it sufficient to always store the event handlers
// as ordinary members using their canonical names, instead of this
// special table?  I have a feeling that's what Macromedia does
// (though I'm not sure).
#if 0
	// This class is just as_object, with an event
	// handler table added.
	struct as_object_with_handlers : public as_object
	{
                // ActionScript event handler table.
                hash<event_id, gnash::as_value>        m_event_handlers;

                // ActionScript event handler.
                void    set_event_handler(event_id id, const as_value& method)
                {
                        // m_event_handlers.push_back(as);
                        //m_event_handlers.set(id, method);
                }

                bool    get_event_handler(event_id id, gnash::as_value* result)
                {
                        //return m_event_handlers.get(id, result);
			return false;
                }
	};
#endif // 0



	/// ActionScript "environment", essentially VM state?
	struct as_environment
	{
		/// Stack of as_values in this environment
		std::vector<as_value>	m_stack;

		as_value	m_global_register[4];

		/// function2 uses this
		std::vector<as_value>	m_local_register;

		/// Movie target. 
		movie*	m_target;

		/// Variables available in this environment
		stringi_hash<as_value>	m_variables;

		/// For local vars.  Use empty names to separate frames.
		struct frame_slot
		{
			tu_string	m_name;
			as_value	m_value;

			frame_slot() {}
			frame_slot(const tu_string& name, const as_value& val) : m_name(name), m_value(val) {}
		};
		std::vector<frame_slot>	m_local_frames;


		as_environment()
			:
			m_target(0)
		{
		}

		movie*	get_target() { return m_target; }
		void	set_target(movie* target) { m_target = target; }

		// stack access/manipulation
		// @@ TODO do more checking on these
		template<class T>
		// stack access/manipulation
		void	push(T val) { push_val(as_value(val)); }
		void	push_val(const as_value& val) { m_stack.push_back(val); }


		/// Pops an as_value off the stack top and return it.
		as_value	pop() { as_value result = m_stack.back(); m_stack.pop_back(); return result; }

		/// Get stack value at the given distance from top.
		//
		/// top(0) is actual stack top
		///
		as_value&	top(int dist) { return m_stack[m_stack.size() - 1 - dist]; }

		/// Get stack value at the given distance from bottom.
		//
		/// bottom(0) is actual stack top
		///
		as_value&	bottom(int index) { return m_stack[index]; }

		/// Drop 'count' values off the top of the stack.
		void	drop(int count) { m_stack.resize(m_stack.size() - count); }

		/// Returns index of top stack element
		int	get_top_index() const { return m_stack.size() - 1; }

		/// \brief
		/// Return the (possibly UNDEFINED) value of the named var.
		/// Variable name can contain path elements.
		///
		as_value get_variable(const tu_string& varname,
			const std::vector<with_stack_entry>& with_stack) const;

		/// Same of the above, but no support for path.
		as_value get_variable_raw(const tu_string& varname,
			const std::vector<with_stack_entry>& with_stack) const;

		/// Given a path to variable, set its value.
		void	set_variable(const tu_string& path, const as_value& val, const std::vector<with_stack_entry>& with_stack);

		/// Same of the above, but no support for path
		void	set_variable_raw(const tu_string& path, const as_value& val, const std::vector<with_stack_entry>& with_stack);

		/// Set/initialize the value of the local variable.
		void	set_local(const tu_string& varname, const as_value& val);
		/// \brief
		/// Add a local var with the given name and value to our
		/// current local frame. 
		///
		/// Use this when you know the var
		/// doesn't exist yet, since it's faster than set_local();
		/// e.g. when setting up args for a function.
		void	add_local(const tu_string& varname, const as_value& val);

		/// Create the specified local var if it doesn't exist already.
		void	declare_local(const tu_string& varname);

		/// Retrieve the named member (variable).
		//
		/// If no member is found under the given name
		/// 'val' is untouched and 'false' is returned.
		/// 
		bool	get_member(const tu_stringi& varname, as_value* val) const;
		void	set_member(const tu_stringi& varname, const as_value& val);

		// Parameter/local stack frame management.
		int	get_local_frame_top() const { return m_local_frames.size(); }
		void	set_local_frame_top(unsigned int t) {
			assert(t <= m_local_frames.size());
			m_local_frames.resize(t);
		}
		void	add_frame_barrier() { m_local_frames.push_back(frame_slot()); }

		// Local registers.
		void	add_local_registers(int register_count)
		{
			m_local_register.resize(m_local_register.size() + register_count);
		}
		void	drop_local_registers(int register_count)
		{
			m_local_register.resize(m_local_register.size() - register_count);
		}
		as_value*	local_register_ptr(int reg);

		// Internal.
		int	find_local(const tu_string& varname) const;
		bool	parse_path(const tu_string& var_path, tu_string* path, tu_string* var) const;
		movie*	find_target(const tu_string& path) const;
		movie*	find_target(const as_value& val) const;

		/// Dump content of the stack using the log_msg function
		void dump_stack()
		{
			for (int i=0, n=m_stack.size(); i<n; i++)
			{
				log_msg("Stack[%d]: %s\n",
					i, m_stack[i].to_string());
			}
		}
	};


	/// Parameters/environment for C functions callable from ActionScript.
	struct fn_call
	{
		as_value* result;
		as_object* this_ptr;
		as_environment* env;
		int nargs;
		int first_arg_bottom_index;

		fn_call(as_value* res_in, as_object* this_in,
				as_environment* env_in,
				int nargs_in, int first_in)
			:
			result(res_in),
			this_ptr(this_in),
			env(env_in),
			nargs(nargs_in),
			first_arg_bottom_index(first_in)
		{
		}

		/// Access a particular argument.
		as_value& arg(int n) const
		{
			assert(n < nargs);
			return env->bottom(first_arg_bottom_index - n);
		}

	};


	//
	// Some handy helpers
	//

	/// Create/hook built-ins.
	void	action_init();

	// Clean up any stray heap stuff we've allocated.
	void	action_clear();

	// Dispatching methods from C++.
	as_value	call_method0(const as_value& method, as_environment* env, as_object* this_ptr);
	as_value	call_method1(
		const as_value& method, as_environment* env, as_object* this_ptr,
		const as_value& arg0);
	as_value	call_method2(
		const as_value& method, as_environment* env, as_object* this_ptr,
		const as_value& arg0, const as_value& arg1);
	as_value	call_method3(
		const as_value& method, as_environment* env, as_object* this_ptr,
		const as_value& arg0, const as_value& arg1, const as_value& arg2);

	///
	/// first_arg_bottom_index is the stack index, from the bottom,
	/// of the first argument.  Subsequent arguments are at *lower*
	/// indices.  E.g. if first_arg_bottom_index = 7, then arg1 is
	/// at env->bottom(7), arg2 is at env->bottom(6), etc.
	///
	as_value call_method(const as_value& method, as_environment* env,
		as_object* this_ptr, // this is ourself
		int nargs, int first_arg_bottom_index);

	const char*	call_method_parsed(
		as_environment* env,
		as_object* this_ptr,
		const char* method_name,
		const char* method_arg_fmt,
		va_list args);

	// tulrich: don't use this!  To register a class constructor,
	// just assign the classname to the constructor function.  E.g.:
	//
	// my_movie->set_member("MyClass", as_value(MyClassConstructorFunction));
	// 
	//void register_as_object(const char* object_name, as_c_function_ptr handler);

	/// Numerical indices for standard member names.  Can use this
	/// to help speed up get/set member calls, by using a switch()
	/// instead of nasty string compares.
	enum as_standard_member
	{
		M_INVALID_MEMBER = -1,
		M_X,
		M_Y,
		M_XSCALE,
		M_YSCALE,
		M_CURRENTFRAME,
		M_TOTALFRAMES,
		M_ALPHA,
		M_VISIBLE,
		M_WIDTH,
		M_HEIGHT,
		M_ROTATION,
		M_TARGET,
		M_FRAMESLOADED,
		M_NAME,
		M_DROPTARGET,
		M_URL,
		M_HIGHQUALITY,
		M_FOCUSRECT,
		M_SOUNDBUFTIME,
		M_XMOUSE,
		M_YMOUSE,
		M_PARENT,
		M_TEXT,
		M_TEXTWIDTH,
		M_TEXTCOLOR,
		M_ONLOAD,

		AS_STANDARD_MEMBER_COUNT
	};

	/// Return the standard enum, if the arg names a standard member.
	/// Returns M_INVALID_MEMBER if there's no match.
	as_standard_member	get_standard_member(const tu_stringi& name);

}	// end namespace gnash


#endif // GNASH_ACTION_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
