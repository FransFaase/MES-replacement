#include <unistd.h>
#include <fcntl.h>

//#define TRACE_ALLOCATIONS 1
//#define CHECK_LOCAL_RESULT
//#define SAFE_CASTING

#define INCLUDED
#include "../RawParser/src/RawParser.c"

void tcc_grammar(non_terminal_dict_p *all_nt)
{
	white_space_grammar(all_nt);
	ident_grammar(all_nt);
	
	HEADER(all_nt)
	
	NT_DEF("primary_expr")
		RULE IDENT PASS
		RULE NTP("int") WS
		//RULE NTP("double") WS
		RULE NTP("char") WS
		RULE NTP("string") WS
		RULE CHAR_WS('(') NTP("expr") CHAR_WS(')')

	NT_DEF("postfix_expr")
		RULE NTP("primary_expr")
		REC_RULEC CHAR_WS('[') NT("expr") CHAR_WS(']') TREE("arrayexp")
		REC_RULEC CHAR_WS('(') NT("assignment_expr") SEQL { CHAIN CHAR_WS(',') } OPTN CHAR_WS(')') TREE("call")
		REC_RULEC CHAR_WS('.') IDENT TREE("field")
		REC_RULEC CHAR('-') CHAR_WS('>') IDENT TREE("fieldderef")
		REC_RULEC CHAR('+') CHAR_WS('+') TREE("post_inc")
		REC_RULEC CHAR('-') CHAR_WS('-') TREE("post_dec")

	NT_DEF("unary_expr")
		RULE CHAR('+') CHAR_WS('+') NT("unary_expr") TREE("pre_inc")
		RULE CHAR('-') CHAR_WS('-') NT("unary_expr") TREE("pre_dec")
		RULE CHAR_WS('&') NT("cast_expr") TREE("address_of")
		RULE CHAR_WS('*') NT("cast_expr") TREE("deref")
		RULE CHAR_WS('+') NT("cast_expr") TREE("plus")
		RULE CHAR_WS('-') NT("cast_expr") TREE("min")
		RULE CHAR_WS('~') NT("cast_expr") TREE("invert")
		RULE CHAR_WS('!') NT("cast_expr") TREE("not")
		RULE KEYWORD("sizeof") CHAR_WS('(') NT("sizeof_type") CHAR_WS(')') TREE("sizeof")
		RULE KEYWORD("sizeof") NT("unary_expr") TREE("sizeof_expr")
		RULE NTP("postfix_expr")

	NT_DEF("sizeof_type")
		//RULE KEYWORD("char") TREE("char")
		//RULE KEYWORD("short") TREE("short")
		RULE KEYWORD("int") TREE("int")
		//RULE KEYWORD("long") TREE("long")
		//RULE KEYWORD("signed") NT("sizeof_type") TREE("signed_type")
		RULE KEYWORD("unsigned") NT("sizeof_type") TREE("unsigned_type")
		//RULE KEYWORD("float") TREE("float")
		RULE KEYWORD("double") NT("sizeof_type") OPTN TREE("double_type")
		//RULE KEYWORD("const") NT("sizeof_type") TREE("const_type")
		//RULE KEYWORD("volatile") NT("sizeof_type") TREE("volatile_type")
		RULE KEYWORD("void") CHAR_WS('*') TREE("void_ptr")
		RULE KEYWORD("struct") IDENT TREE("structdecl")
		RULE IDENT PASS
		REC_RULEC WS CHAR_WS('*') TREE("pointdecl")

	NT_DEF("cast_expr")
		RULE CHAR_WS('(') NT("abstract_declaration") CHAR_WS(')') NT("cast_expr") TREE("cast")
		RULE NTP("unary_expr")

	NT_DEF("l_expr1")
		RULE NTP("cast_expr")
		REC_RULEC WS CHAR_WS('*') NT("cast_expr") TREE("times")
		REC_RULEC WS CHAR_WS('/') NT("cast_expr") TREE("div")
		REC_RULEC WS CHAR_WS('%') NT("cast_expr") TREE("mod")

	NT_DEF("l_expr2")
		RULE NTP("l_expr1")
		REC_RULEC WS CHAR_WS('+') NT("l_expr1") TREE("add")
		REC_RULEC WS CHAR_WS('-') NT("l_expr1") TREE("sub")

	NT_DEF("l_expr3")
		RULE NTP("l_expr2")
		REC_RULEC WS CHAR('<') CHAR_WS('<') NT("l_expr2") TREE("ls")
		REC_RULEC WS CHAR('>') CHAR_WS('>') NT("l_expr2") TREE("rs")

	NT_DEF("l_expr4")
		RULE NTP("l_expr3")
		REC_RULEC WS CHAR('<') CHAR_WS('=') NT("l_expr3") TREE("le")
		REC_RULEC WS CHAR('>') CHAR_WS('=') NT("l_expr3") TREE("ge")
		REC_RULEC WS CHAR_WS('<') NT("l_expr3") TREE("lt")
		REC_RULEC WS CHAR_WS('>') NT("l_expr3") TREE("gt")
		REC_RULEC WS CHAR('=') CHAR_WS('=') NT("l_expr3") TREE("eq")
		REC_RULEC WS CHAR('!') CHAR_WS('=') NT("l_expr3") TREE("ne")

	NT_DEF("l_expr5")
		RULE NTP("l_expr4")
		REC_RULEC WS CHAR_WS('^') NT("l_expr4") TREE("bexor")

	NT_DEF("l_expr6")
		RULE NTP("l_expr5")
		REC_RULEC WS CHAR_WS('&') NT("l_expr5") TREE("land")

	NT_DEF("l_expr7")
		RULE NTP("l_expr6")
		REC_RULEC WS CHAR_WS('|') NT("l_expr6") TREE("lor")

	NT_DEF("l_expr8")
		RULE NTP("l_expr7")
		REC_RULEC WS CHAR('&') CHAR_WS('&') NT("l_expr7") TREE("and")

	NT_DEF("l_expr9")
		RULE NTP("l_expr8")
		REC_RULEC WS CHAR('|') CHAR_WS('|') NT("l_expr8") TREE("or")

	NT_DEF("conditional_expr")
		RULE NT("l_expr9") WS CHAR_WS('?') NT("l_expr9") WS CHAR_WS(':') NT("conditional_expr") TREE("if_expr")
		RULE NTP("l_expr9")

	NT_DEF("assignment_expr")
		RULE NT("unary_expr") WS NT("assignment_operator") WS NT("assignment_expr") TREE("assignment")
		RULE NTP("conditional_expr")

	NT_DEF("assignment_operator")
		RULE CHAR_WS('=') TREE("ass")
		RULE CHAR('*') CHAR_WS('=') TREE("times_ass")
		RULE CHAR('/') CHAR_WS('=') TREE("div_ass")
		RULE CHAR('%') CHAR_WS('=') TREE("mod_ass")
		RULE CHAR('+') CHAR_WS('=') TREE("add_ass")
		RULE CHAR('-') CHAR_WS('=') TREE("sub_ass")
		RULE CHAR('<') CHAR('<') CHAR_WS('=') TREE("sl_ass")
		RULE CHAR('>') CHAR('>') CHAR_WS('=') TREE("sr_ass")
		RULE CHAR('&') CHAR_WS('=') TREE("and_ass")
		RULE CHAR('|') CHAR_WS('=') TREE("or_ass")
		RULE CHAR('^') CHAR_WS('=') TREE("exor_ass")

	NT_DEF("expr")
		RULE NT("assignment_expr") SEQL { CHAIN CHAR_WS(',') } PASS

	NT_DEF("constant_expr")
		RULE NTP("conditional_expr")

	NT_DEF("declaration")
		RULE
		{ GROUPING
			RULE NTP("storage_class_specifier")
		} SEQL OPTN ADD_CHILD
		NT("type_specifier")
		{ GROUPING
			RULE NT("func_declarator") CHAR_WS('(')
			{ GROUPING
				RULE NTP("parameter_declaration_list") OPTN
				//RULE KEYWORD("void") TREE("voidx")
			} ADD_CHILD
			CHAR_WS(')')
			{ GROUPING
				RULE CHAR_WS(';') TREE("forward")
				RULE CHAR_WS('{') NTP("decl_or_stat") CHAR_WS('}')
			} ADD_CHILD TREE("new_style") WS
			RULE
			{ GROUPING
				RULE NT("declarator")
				{ GROUPING
					RULE WS CHAR_WS('=') NTP("initializer")
				} OPTN ADD_CHILD TREE("decl_init")
			} SEQL OPTN ADD_CHILD { CHAIN CHAR_WS(',') } CHAR_WS(';') TREE("decl")
		}

	NT_DEF("storage_class_specifier")
		RULE KEYWORD("typedef") TREE("typedef")
		RULE KEYWORD("extern") TREE("extern")
		RULE KEYWORD("inline") TREE("inline")
		RULE KEYWORD("static") TREE("static")
		//RULE KEYWORD("auto") TREE("auto")
		//RULE KEYWORD("register") TREE("register")
		//RULE KEYWORD("const") TREE("const")
		//RULE KEYWORD("volatile") TREE("volatile")

	NT_DEF("type_specifier")
		RULE KEYWORD("const") KEYWORD("char") TREE("const_char")
		RULE KEYWORD("char") KEYWORD("const") TREE("const_char")
		RULE KEYWORD("const") KEYWORD("unsigned") KEYWORD("char") TREE("const_u_char")
		RULE KEYWORD("unsigned") KEYWORD("char") TREE("u_char")
		RULE KEYWORD("unsigned") KEYWORD("short") TREE("u_short")
		RULE KEYWORD("unsigned") KEYWORD("long") KEYWORD("long") TREE("u_long_long_int")
		RULE KEYWORD("unsigned") KEYWORD("long") TREE("u_long_int")
		RULE KEYWORD("unsigned") KEYWORD("int") TREE("u_int")
		RULE KEYWORD("unsigned") TREE("u_int")
		RULE KEYWORD("char") TREE("char")
		RULE KEYWORD("short") TREE("short")
		RULE KEYWORD("const") KEYWORD("int") TREE("const_int")
		RULE KEYWORD("int") TREE("int")
		RULE KEYWORD("long") KEYWORD("double") TREE("long_double")
		RULE KEYWORD("long") KEYWORD("long") TREE("long_long_int")
		RULE KEYWORD("long") TREE("long")
		//RULE KEYWORD("float") TREE("float")
		RULE KEYWORD("double") TREE("double")
		RULE KEYWORD("const") KEYWORD("void") TREE("const_void")
		RULE KEYWORD("void") TREE("void")
		RULE KEYWORD("const") IDENT TREE("const")
		RULE KEYWORD("const") NTP("struct_or_union_specifier")
		RULE NTP("struct_or_union_specifier")
		RULE NTP("enum_specifier")
		RULE IDENT PASS

	NT_DEF("struct_or_union_specifier")
		RULE KEYWORD("struct") IDENT_OPT
		{ GROUPING
			RULE CHAR_WS('{')
			{ GROUPING
				RULE NTP("struct_declaration_or_anon")
			} SEQL ADD_CHILD
			CHAR_WS('}') PASS
		} OPTN ADD_CHILD TREE("struct")
		RULE KEYWORD("union") IDENT_OPT
		{ GROUPING
			RULE CHAR_WS('{')
			{ GROUPING
				RULE NTP("struct_declaration_or_anon")
			} SEQL ADD_CHILD
			CHAR_WS('}') PASS
		} OPTN ADD_CHILD TREE("union")

	NT_DEF("struct_declaration_or_anon")
		RULE NTP("struct_or_union_specifier") CHAR_WS(';')
		RULE NTP("struct_declaration")

	NT_DEF("struct_declaration")
		RULE NT("type_specifier") NT("declarator") SEQL { CHAIN CHAR_WS(',') } CHAR_WS(';') TREE("struct_declaration")
		//RULE NT("type_specifier") NT("struct_declaration") TREE("type")
		//RULE NT("struct_declarator") SEQL { CHAIN CHAR_WS(',') } CHAR_WS(';') TREE("strdec")

	//NT_DEF("struct_declarator")
	//	RULE NT("declarator")
	//	{ GROUPING
	//		RULE CHAR_WS(':') NT("constant_expr")
	//	} OPTN ADD_CHILD TREE("record_field")

	NT_DEF("enum_specifier")
		RULE KEYWORD("enum") IDENT_OPT CHAR_WS('{') NT("enumerator") SEQL { CHAIN CHAR_WS(',') } CHAR_WS('}') TREE("enum")

	NT_DEF("enumerator")
		RULE IDENT
		{ GROUPING
			RULE CHAR_WS('=') NTP("constant_expr")
		} OPTN ADD_CHILD TREE("enumerator")

	NT_DEF("func_declarator")
		RULE CHAR_WS('*')
		{ GROUPING
			RULE KEYWORD("const") TREE("const")
		} OPTN ADD_CHILD NT("func_declarator") TREE("pointdecl")
		RULE CHAR_WS('(') NT("func_declarator") CHAR_WS(')')
		RULE IDENT PASS

	NT_DEF("declarator")
		RULE CHAR_WS('*')
		{ GROUPING
			RULE KEYWORD("const") TREE("const")
		} OPTN ADD_CHILD NT("declarator") TREE("pointdecl")
		RULE CHAR_WS('(') NT("declarator") CHAR_WS(')') TREE("brackets")
		RULE WS IDENT PASS
		REC_RULEC CHAR_WS('[') NT("constant_expr") OPTN CHAR_WS(']') TREE("array")
		REC_RULEC CHAR_WS('(') NT("abstract_declaration_list") OPTN CHAR_WS(')') TREE("function")

	NT_DEF("abstract_declaration_list")
		RULE
			NT("abstract_declaration") SEQL BACK_TRACKING { CHAIN CHAR_WS(',') }
			{ GROUPING
				RULE CHAR_WS(',') CHAR('.') CHAR('.') CHAR_WS('.') TREE("varargs")
			} OPTN ADD_CHILD TREE("abstract_declaration_list")

	NT_DEF("parameter_declaration_list")
		RULE
			NT("parameter_declaration") SEQL BACK_TRACKING { CHAIN CHAR_WS(',') }
			{ GROUPING
				RULE CHAR_WS(',') CHAR('.') CHAR('.') CHAR_WS('.') TREE("varargs")
			} OPTN ADD_CHILD TREE("parameter_declaration_list")

	NT_DEF("ident_list")
		RULE IDENT
		{ GROUPING
			RULE CHAR_WS(',')
			{ GROUPING
				RULE CHAR('.') CHAR('.') CHAR_WS('.') TREE("varargs")
				RULE NT("ident_list") TREE("ident_list")
			}
		} OPTN ADD_CHILD TREE("ident_list")

	NT_DEF("parameter_declaration")
		RULE NT("type_specifier")
		{ GROUPING
			RULE NTP("declarator")
			RULE NTP("abstract_declarator")
		} ADD_CHILD TREE("parameter_declaration")

	NT_DEF("abstract_declaration")
		RULE NT("type_specifier")
		{ GROUPING
			RULE NTP("declarator")
			RULE NTP("abstract_declarator")
		} ADD_CHILD TREE("abstract_declarator")
		//RULE NT("type_specifier") NT("parameter_declaration") TREE("type")
		//RULE NTP("abstract_declarator")

	NT_DEF("abstract_declarator")
		RULE CHAR_WS('*')
		{ GROUPING
			RULE KEYWORD("const") TREE("const")
		} OPTN ADD_CHILD NT("abstract_declarator") TREE("abs_pointdecl")
		RULE CHAR_WS('(') NT("abstract_declarator") CHAR_WS(')') TREE("abs_brackets")
		RULE
		REC_RULEC CHAR_WS('[') NT("constant_expr") OPTN CHAR_WS(']') TREE("abs_array")
		REC_RULEC CHAR_WS('(') NT("parameter_declaration_list") CHAR_WS(')') TREE("abs_func")

	NT_DEF("initializer")
		RULE NTP("assignment_expr")
		RULE CHAR_WS('{') NT("initializer") SEQL { CHAIN CHAR_WS(',') } CHAR(',') OPTN WS CHAR_WS('}') TREE("initializer")

	NT_DEF("decl_or_stat")
		RULE NT("declaration") SEQL OPTN NT("statement") SEQL OPTN TREE("decl_or_stat")

	NT_DEF("statement")
		RULE
		{ GROUPING
			RULE
			{ GROUPING
				RULE IDENT TREE("open_label")
				RULE KEYWORD("case") NT("constant_expr") TREE("case")
				RULE KEYWORD("default") TREE("default")
			} ADD_CHILD CHAR_WS(':') NT("statement") TREE("label")
			RULE CHAR_WS('{') NTP("decl_or_stat") CHAR_WS('}')
		}
		RULE
		{ GROUPING
			RULE NTP("expr") OPTN CHAR_WS(';')
			RULE KEYWORD("if") WS CHAR_WS('(') NT("expr") CHAR_WS(')') NT("statement")
			{ GROUPING
				RULE KEYWORD("else") NTP("statement")
			} OPTN ADD_CHILD TREE("if")
			RULE KEYWORD("switch") WS CHAR_WS('(') NT("expr") CHAR_WS(')') NT("statement") TREE("switch")
			RULE KEYWORD("while") WS CHAR_WS('(') NT("expr") CHAR_WS(')') NT("statement") TREE("while")
			RULE KEYWORD("do") NT("statement") KEYWORD("while") WS CHAR_WS('(') NT("expr") CHAR_WS(')') CHAR_WS(';') TREE("do")
			RULE KEYWORD("for") WS CHAR_WS('(') NT("expr") OPTN CHAR_WS(';')
			{ GROUPING
				RULE WS NTP("expr")
			} OPTN ADD_CHILD CHAR_WS(';')
			{ GROUPING
				RULE WS NTP("expr")
			} OPTN ADD_CHILD CHAR_WS(')') NT("statement") TREE("for")
			RULE KEYWORD("goto") IDENT CHAR_WS(';') TREE("goto")
			//RULE KEYWORD("continue") CHAR_WS(';') TREE("cont")
			//RULE KEYWORD("break") CHAR_WS(';') TREE("break")
			RULE KEYWORD("return") NT("expr") OPTN CHAR_WS(';') TREE("ret")
		}

	NT_DEF("root")
		RULE
		WS
		{ GROUPING
			RULE NT("declaration")
		} SEQL OPTN END PASS

}

void dump_tree(tree_p tree)
{
	printf("%s(\n", tree->tree_name);
	file_ostream_t ostream;
	file_ostream_init(&ostream, stdout);
	for (int i = 0; i < tree->nr_children; i++)
	{
		printf("  %d: ", i);
		if (tree->children[i].data == NULL)
			printf("NULL");
		else
			tree->children[i].print(tree->children[i].data, &ostream.ostream);
		printf("\n");
	}
	printf(")");
}

int indent_depth = 0;
void newline() { printf("\n"); for (int i = 0; i < indent_depth; i++) printf("\t"); }


#define NR_CHILDREN(N) if (tree->nr_children != (N)) { printf("$%s:%d ", tree->tree_name, N); dump_tree(tree); return; }


void print_char(char ch, char del)
{
	if (ch == '\0')
		printf("\\0");
	else if (ch == del)
		printf("\\%c", del);
	else if (ch == '\n')
		printf("\\n");
	else if (ch == '\r')
		printf("\\r");
	else if (ch == '\\')
		printf("\\\\");
	else
		printf("%c", ch);
}

int longest_string = 0;

#define print(T) print_line(T, __LINE__)
void print_line(tree_node_p tree_node, int line)
{
	if (tree_node == NULL)
	{
		printf("$NULL(%d)", line);
		return;
	}
	if (tree_node->type_name == NULL)
	{
		printf("$type_name=NULL_%p", tree_node);
		return;
	}
	
	if (tree_node->type_name == ident_node_type)
	{
		printf("%s", ((ident_p)tree_node)->name);
		return;
	}
	if (tree_node->type_name == char_node_type)
	{
		printf("'");
		print_char(((char_node_p)tree_node)->ch, '\'');
		printf("'");
		return;
	}
	if (tree_node->type_name == int_node_type)
	{
		printf("%lld", ((int_node_p)tree_node)->value);
		return;
	}
	if (tree_node->type_name == string_node_type)
	{
		printf("\"");
		string_node_p string_node = ((string_node_p)tree_node);
		for (size_t i = 0; i < string_node->length - 1; i++)
			print_char(string_node->str[i], '"');
		printf("\"");
		if (string_node->length > longest_string)
			longest_string = string_node->length;
		return;
	}
	if (tree_node->type_name == tree_node_type)
	{
		tree_p tree = (tree_p)tree_node;
		
		if (tree->tree_name == list_type)
		{
			for (int i = 0; i < tree->nr_children; i++)
				print((tree_node_p)tree->children[i].data);
		}
		else if (strcmp(tree->tree_name, "new_style") == 0)
		{
			newline();
			//printf("new_style: ");
			NR_CHILDREN(5)
			if (tree->children[0].data != NULL)
				print((tree_node_p)tree->children[0].data);
			for (int i = 1; i < tree->nr_children; i++)
			{
				printf(" ");
				print((tree_node_p)tree->children[i].data);
			}
		}
		else if (strcmp(tree->tree_name, "decl") == 0)
		{
			newline();
			//printf("decl: ");
			NR_CHILDREN(3)
			if (tree->children[0].data != NULL)
			{
				print((tree_node_p)tree->children[0].data);
				printf(" ");
			}
			print((tree_node_p)tree->children[1].data);
			if (tree->children[2].data != NULL)
			{
				printf(" ");
				print((tree_node_p)tree->children[2].data);
			}
			printf(";");
		}
		else if (strcmp(tree->tree_name, "parameter_declaration_list") == 0)
		{
			printf("(");
			tree_p args_tree = (tree_p)tree->children[0].data;
			for (int i = 0; i < args_tree->nr_children; i++)
			{
				if (i > 0)
					printf(", ");
				print((tree_node_p)args_tree->children[i].data);
			}
			if (tree->children[1].data != NULL)
				printf(", ...");
			printf(")");
		}
		else if (strcmp(tree->tree_name, "parameter_declaration") == 0)
		{
			NR_CHILDREN(2)
			print((tree_node_p)tree->children[0].data);
			if (tree->children[1].data != NULL)
			{
				printf(" ");
				print((tree_node_p)tree->children[1].data);
			}
		}
		else if (strcmp(tree->tree_name, "abstract_declaration_list") == 0)
		{
			printf("(");
			tree_p args_tree = (tree_p)tree->children[0].data;
			for (int i = 0; i < args_tree->nr_children; i++)
			{
				if (i > 0)
					printf(", ");
				print((tree_node_p)args_tree->children[i].data);
			}
			if (tree->children[1].data != NULL)
				printf(", ...");
			printf(")");
		}
		else if (strcmp(tree->tree_name, "abstract_declarator") == 0)
		{
			NR_CHILDREN(2)
			print((tree_node_p)tree->children[0].data);
			if (tree->children[1].data != NULL)
			{
				printf(" ");
				print((tree_node_p)tree->children[1].data);
			}
		}
		else if (strcmp(tree->tree_name, "initializer") == 0)
		{
			NR_CHILDREN(1)
			printf("{");
			if (tree->children[0].data != 0)
			{
				tree_p values = (tree_p)tree->children[0].data;
				for (int i = 0; i < values->nr_children; i++)
				{
					if (i > 0)
						printf(", ");
					print((tree_node_p)values->children[i].data);
				}
			}
			printf("}");
		}
		else if (strcmp(tree->tree_name, "structdecl") == 0)
		{
			NR_CHILDREN(1)
			printf("struct ");
			print((tree_node_p)tree->children[0].data);
		}
		else if (strcmp(tree->tree_name, "pointdecl") == 0)
		{
			if (tree->nr_children == 2)
			{
				printf("*");
				if (tree->children[0].data != 0)
					printf("const ");
				print((tree_node_p)tree->children[1].data);
			}
			else
			{
				NR_CHILDREN(1)
				print((tree_node_p)tree->children[0].data);
				printf("*");
			}
		}
		else if (strcmp(tree->tree_name, "struct") == 0)
		{
			NR_CHILDREN(2)
			printf("struct ");
			if (tree->children[0].data != NULL)
			{
				tree_node_p first = (tree_node_p)tree->children[0].data;
				if (first->type_name != ident_node_type)
					printf("$%s", first->type_name);
				else
					printf("%s", ((ident_p)first)->name);
			}
			if (tree->children[1].data != NULL)
			{
				newline();
				printf("{");
				indent_depth++;
				tree_p second = CAST(tree_p, tree->children[1].data);
				for (int i = 0; i < second->nr_children; i++)
				{
					newline();
					tree_p decl = CAST(tree_p, second->children[i].data);
					if (   strcmp(decl->tree_name, "struct") == 0
						|| strcmp(decl->tree_name, "union") == 0)
					{
						print((tree_node_p)decl);
					}
					else
					{
						print((tree_node_p)decl->children[0].data);
						printf(" ");
						print((tree_node_p)decl->children[1].data);
						printf(";");
					}
				}
				indent_depth--;
				newline();
				printf("}");
			}
		}
		else if (strcmp(tree->tree_name, "union") == 0)
		{
			NR_CHILDREN(2)
			printf("union ");
			if (tree->children[0].data != NULL)
			{
				tree_node_p first = (tree_node_p)tree->children[0].data;
				if (first->type_name != ident_node_type)
					printf("$%s", first->type_name);
				else
					printf("%s", ((ident_p)first)->name);
			}
			if (tree->children[1].data != NULL)
			{
				newline();
				indent_depth++;
				printf("{");
				tree_p second = CAST(tree_p, tree->children[1].data);
				for (int i = 0; i < second->nr_children; i++)
				{
					tree_p decl = CAST(tree_p, second->children[i].data);
					newline();
					print((tree_node_p)decl->children[0].data);
					printf(" ");
					print((tree_node_p)decl->children[1].data);
					printf(";");
				}
				indent_depth--;
				newline();
				printf("}");
			}
		}
		else if (strcmp(tree->tree_name, "struct_declaration") == 0)
		{
			NR_CHILDREN(2)
			newline();
			print((tree_node_p)tree->children[0].data);
			printf(" ");
			print((tree_node_p)tree->children[1].data);
			printf(";");
		}
		else if (strcmp(tree->tree_name, "enum") == 0)
		{
			NR_CHILDREN(2)
			printf("enum ");
			if (tree->children[0].data != 0)
				print((tree_node_p)tree->children[0].data);
			printf("{");
			indent_depth++;
			tree_p enums = CAST(tree_p, tree->children[1].data);
			for (int i = 0; i < enums->nr_children; i++)
			{
				tree_p enumerator = CAST(tree_p, enums->children[i].data);
				newline();
				print((tree_node_p)enumerator->children[0].data);
				if (enumerator->children[1].data != NULL)
				{
					printf(" = ");
					print((tree_node_p)enumerator->children[1].data);
				}
				printf(";");
			}
			indent_depth--;
			newline();
			printf("}");
		}
		else if (strcmp(tree->tree_name, "array") == 0)
		{
			NR_CHILDREN(2)
			print((tree_node_p)tree->children[0].data);
			printf("[");
			if (tree->children[1].data != NULL)
				print((tree_node_p)tree->children[1].data);
			printf("]");
		}
		else if (strcmp(tree->tree_name, "function") == 0)
		{
			NR_CHILDREN(2)
			print((tree_node_p)tree->children[0].data);
			print((tree_node_p)tree->children[1].data);
		}
		else if (strcmp(tree->tree_name, "abs_pointdecl") == 0)
		{
			NR_CHILDREN(2)
			printf("*");
			if (tree->children[0].data != NULL)
			{
				printf("$abs_pointdeclError0");
				//tree_p first = CAST(tree_p, tree->children[0].data);
				//if (tree->children[0].data != NULL)
				//	printf("$abs_pointdeclError");
			}
			if (tree->children[0].data != NULL)
				printf("$abs_pointdeclError1");
		}
		else if (strcmp(tree->tree_name, "brackets") == 0)
		{
			NR_CHILDREN(1)
			printf("(");
			print((tree_node_p)tree->children[0].data);
			printf(")");
		}
		else if (strcmp(tree->tree_name, "forward") == 0)
			printf("; /* forward */");
		else if (strcmp(tree->tree_name, "decl_init") == 0)
		{
			NR_CHILDREN(2)
			print((tree_node_p)tree->children[0].data);
			if (tree->children[1].data != NULL)
			{
				printf(" = ");
				tree_p initializer = (tree_p)tree->children[1].data;
				if (tree->children[1].data != NULL)
					print((tree_node_p)tree->children[1].data);
			}
		}
		else if (strcmp(tree->tree_name, "const") == 0 && tree->nr_children == 1)
		{
			printf("const ");
			print((tree_node_p)tree->children[0].data);
		}
		else if (   strcmp(tree->tree_name, "typedef") == 0
				 || strcmp(tree->tree_name, "extern") == 0
				 || strcmp(tree->tree_name, "static") == 0
				 || strcmp(tree->tree_name, "inline") == 0
				 || strcmp(tree->tree_name, "const") == 0
				 || strcmp(tree->tree_name, "float") == 0
				 || strcmp(tree->tree_name, "double") == 0
				 || strcmp(tree->tree_name, "char") == 0
				 || strcmp(tree->tree_name, "u_char") == 0
				 || strcmp(tree->tree_name, "const_char") == 0
				 || strcmp(tree->tree_name, "const_u_char") == 0
				 || strcmp(tree->tree_name, "const_void") == 0
				 || strcmp(tree->tree_name, "void") == 0
				 || strcmp(tree->tree_name, "int") == 0
				 || strcmp(tree->tree_name, "short") == 0
				 || strcmp(tree->tree_name, "const_int") == 0
				 || strcmp(tree->tree_name, "u_int") == 0
				 || strcmp(tree->tree_name, "u_long_long_int") == 0
				 || strcmp(tree->tree_name, "u_long_int") == 0
				 || strcmp(tree->tree_name, "long_long_int") == 0
				 || strcmp(tree->tree_name, "long") == 0
				 || strcmp(tree->tree_name, "u_short") == 0
				 || strcmp(tree->tree_name, "long_double") == 0)
		{
			NR_CHILDREN(0)
			printf("%s ", tree->tree_name);
		}
		else if (strcmp(tree->tree_name, "double_type") == 0)
		{
			NR_CHILDREN(1);
			printf("double ");
			if (tree->children[0].data != NULL)
				print((tree_node_p)tree->children[0].data);
		}
		else if (strcmp(tree->tree_name, "unsigned_type") == 0)
		{
			NR_CHILDREN(1);
			printf("unsigned ");
			if (tree->children[0].data != NULL)
				print((tree_node_p)tree->children[0].data);
		}
		else if (strcmp(tree->tree_name, "decl_or_stat") == 0)
		{
			NR_CHILDREN(2)
			newline();
			printf("{");
			indent_depth++;
			if (tree->children[0].data != NULL)
			{
				newline();
				printf("// declarations");
				tree_p first = CAST(tree_p, tree->children[0].data);
				for (int i = 0; i < first->nr_children; i++)
				{
					print((tree_node_p)first->children[i].data);
				}
			}
			if (tree->children[1].data != NULL)
			{
				newline();
				printf("// statements");
				tree_p second = CAST(tree_p, tree->children[1].data);
				for (int i = 0; i < second->nr_children; i++)
				{
					newline();
					if (second->children[i].data != 0)
						print((tree_node_p)second->children[i].data);
				}
			}
			indent_depth--;
			newline();
			printf("}");
		}
		else if (strcmp(tree->tree_name, "ret") == 0)
		{
			NR_CHILDREN(1)
			printf("return");
			if (tree->children[0].data != NULL)
			{
				printf(" ");
				print((tree_node_p)tree->children[0].data);
			}
			printf(";");
		}
		else if (strcmp(tree->tree_name, "break") == 0)
		{
			NR_CHILDREN(0)
			printf("break;");
		}
		else if (strcmp(tree->tree_name, "cont") == 0)
		{
			NR_CHILDREN(0)
			printf("continue;");
		}
		else if (strcmp(tree->tree_name, "switch") == 0)
		{
			NR_CHILDREN(2)
			printf("switch (");
			print((tree_node_p)tree->children[0].data);
			printf(")");
			newline();
			print((tree_node_p)tree->children[1].data);
		}
		else if (strcmp(tree->tree_name, "label") == 0)
		{
			NR_CHILDREN(2)
			print((tree_node_p)tree->children[0].data);
			printf(": ");
			if (tree->children[1].data != NULL)
				print((tree_node_p)tree->children[1].data);
		}
		else if (strcmp(tree->tree_name, "open_label") == 0)
		{
			NR_CHILDREN(1)
			print((tree_node_p)tree->children[0].data);
		}
		else if (strcmp(tree->tree_name, "case") == 0)
		{
			NR_CHILDREN(1)
			printf("case ");
			print((tree_node_p)tree->children[0].data);
		}
		else if (strcmp(tree->tree_name, "default") == 0)
		{
			NR_CHILDREN(0)
			printf("default ");
		}
		else if (strcmp(tree->tree_name, "if") == 0)
		{
			NR_CHILDREN(3)
			printf("if (");
			print((tree_node_p)tree->children[0].data);
			printf(") ");
			if (tree->children[1].data != NULL)
				print((tree_node_p)tree->children[1].data);
			else
				printf("/*empty*/;");
			if (tree->children[2].data != NULL)
			{
				newline();
				printf("else ");
				print((tree_node_p)tree->children[2].data);
			}
		}
		else if (strcmp(tree->tree_name, "while") == 0)
		{
			NR_CHILDREN(2)
			printf("while (");
			print((tree_node_p)tree->children[0].data);
			printf(")");
			if (tree->children[1].data != 0)
				print((tree_node_p)tree->children[1].data);
			else
				printf(" /*empty*/;");
		}
		else if (strcmp(tree->tree_name, "do") == 0)
		{
			NR_CHILDREN(2)
			printf("do");
			print((tree_node_p)tree->children[0].data);
			printf(" while (");
			print((tree_node_p)tree->children[1].data);
			printf(")");
		}
		else if (strcmp(tree->tree_name, "for") == 0)
		{
			NR_CHILDREN(4)
			printf("for (");
			if (tree->children[0].data)
				print((tree_node_p)tree->children[0].data);
			printf("; ");
			if (tree->children[1].data)
				print((tree_node_p)tree->children[1].data);
			printf("; ");
			if (tree->children[2].data)
				print((tree_node_p)tree->children[2].data);
			printf(") ");
			if (tree->children[3].data != NULL)
				print((tree_node_p)tree->children[3].data);
			else
				printf("/*empty*/;");
		}
		else if (strcmp(tree->tree_name, "assignment") == 0)
		{
			NR_CHILDREN(3);
			print((tree_node_p)tree->children[0].data);
			printf(" ");
			tree_p ass_oper = CAST(tree_p, tree->children[1].data);
			     if (strcmp(ass_oper->tree_name, "ass") == 0) printf("=");
			else if (strcmp(ass_oper->tree_name, "sub_ass") == 0) printf("-=");
			else if (strcmp(ass_oper->tree_name, "add_ass") == 0) printf("+=");
			else if (strcmp(ass_oper->tree_name, "or_ass") == 0) printf("|=");
			else if (strcmp(ass_oper->tree_name, "and_ass") == 0) printf("&=");
			else if (strcmp(ass_oper->tree_name, "exor_ass") == 0) printf("^=");
			else if (strcmp(ass_oper->tree_name, "times_ass") == 0) printf("*=");
			else if (strcmp(ass_oper->tree_name, "div_ass") == 0) printf("/=");
			else if (strcmp(ass_oper->tree_name, "mod_ass") == 0) printf("%%=");
			else if (strcmp(ass_oper->tree_name, "sl_ass") == 0) printf("<<=");
			else if (strcmp(ass_oper->tree_name, "sr_ass") == 0) printf(">>=");
			else printf("$%s ", ass_oper->tree_name);
			printf(" ");
			print((tree_node_p)tree->children[2].data);
		}
		else if (strcmp(tree->tree_name, "goto") == 0)
		{
			NR_CHILDREN(1);
			printf("goto ");
			print((tree_node_p)tree->children[0].data);
			printf(";");
		}
		else if (strcmp(tree->tree_name, "label") == 0)
		{
			NR_CHILDREN(2);
			print((tree_node_p)tree->children[0].data);
			printf(": ");
			print((tree_node_p)tree->children[1].data);
		}
		else if (strcmp(tree->tree_name, "if_expr") == 0)
		{
			NR_CHILDREN(3);
			print((tree_node_p)tree->children[0].data);
			printf(" ? ");
			print((tree_node_p)tree->children[1].data);
			printf(" : ");
			print((tree_node_p)tree->children[2].data);
		}
		else if (strcmp(tree->tree_name, "call") == 0)
		{
			NR_CHILDREN(2);
			print((tree_node_p)tree->children[0].data);
			printf("(");
			if (tree->children[1].data != NULL)
			{
				tree_p args = CAST(tree_p, tree->children[1].data);
				for (int i = 0; i < args->nr_children; i++)
				{
					if (i > 0)
						printf(", ");
					print((tree_node_p)args->children[i].data);
				}
			}
			printf(")");
		}
		else if (strcmp(tree->tree_name, "arrayexp") == 0)
		{
			NR_CHILDREN(2);
			print((tree_node_p)tree->children[0].data);
			printf("[");
			print((tree_node_p)tree->children[1].data);
			printf("]");
		}
		else if (strcmp(tree->tree_name, "field") == 0)
		{
			NR_CHILDREN(2);
			print((tree_node_p)tree->children[0].data);
			printf(".");
			print((tree_node_p)tree->children[1].data);
		}
		else if (strcmp(tree->tree_name, "fieldderef") == 0)
		{
			NR_CHILDREN(2);
			print((tree_node_p)tree->children[0].data);
			printf("->");
			print((tree_node_p)tree->children[1].data);
		}
		else if (strcmp(tree->tree_name, "cast") == 0)
		{
			NR_CHILDREN(2);
			printf("(");
			print((tree_node_p)tree->children[0].data);
			printf(")(");
			print((tree_node_p)tree->children[1].data);
			printf(")");
		}
#define OPERATOR(N,O) else if (strcmp(tree->tree_name, N) == 0) { printf("("); print((tree_node_p)tree->children[0].data); printf(" %s ", O); print((tree_node_p)tree->children[1].data); printf(")"); }
		OPERATOR("times", "*")
		OPERATOR("div", "/")
		OPERATOR("mod", "%")
		OPERATOR("sub", "-")
		OPERATOR("add", "+")
		OPERATOR("ls", "<<")
		OPERATOR("rs", ">>")
		OPERATOR("eq", "==")
		OPERATOR("ne", "!=")
		OPERATOR("lt", "<")
		OPERATOR("le", "<=")
		OPERATOR("gt", ">")
		OPERATOR("ge", ">=")
		OPERATOR("or", "|")
		OPERATOR("bexor", "^")
		OPERATOR("land", "&&")
		OPERATOR("lor", "||")
#define MONOOPERATOR(N,O) else if (strcmp(tree->tree_name, N) == 0) { printf("(%s ", O); print((tree_node_p)tree->children[0].data); printf(")"); }
		else if (   strcmp(tree->tree_name, "sizeof") == 0
				 || strcmp(tree->tree_name, "sizeof_expr") == 0)
		{
			NR_CHILDREN(1)
			printf("sizeof(");
			print((tree_node_p)tree->children[0].data);
			printf(")");
		}
		else if (strcmp(tree->tree_name, "post_dec") == 0)
		{
			NR_CHILDREN(1);
			printf("(");
			print((tree_node_p)tree->children[0].data);
			printf(")--");
		}
		else if (strcmp(tree->tree_name, "post_inc") == 0)
		{
			NR_CHILDREN(1);
			printf("(");
			print((tree_node_p)tree->children[0].data);
			printf(")++");
		}
		MONOOPERATOR("not", "!")
		MONOOPERATOR("invert", "~")
		MONOOPERATOR("address_of", "&")
		MONOOPERATOR("min", "-")
		MONOOPERATOR("plus", "+")
		MONOOPERATOR("deref", "*")
		MONOOPERATOR("pre_inc", "++")
		MONOOPERATOR("pre_dec", "--")
		else
			printf("$$%s(%d) ", tree->tree_name, tree->nr_children);
		return;
	}
	printf("$_type_name$%s ", tree_node->type_name);
}

int main(int argc, char *argv[])
{
	//debug_parse = TRUE;
	//debug_nt = TRUE;
	//debug_nt_result = TRUE;
	
	file_ostream_t debug_ostream;
	file_ostream_init(&debug_ostream, stdout);
	stdout_stream = &debug_ostream.ostream;
	
	non_terminal_dict_p all_nt = NULL;

	white_space_grammar(&all_nt);
	ident_grammar(&all_nt);
	char_grammar(&all_nt);
	string_grammar(&all_nt);
	int_grammar(&all_nt);
	tcc_grammar(&all_nt);
	
	int fh = open("tcc.c", O_RDONLY);
	if (fh < 0)
	{
		fprintf(stderr, "Failed to open out.txt\n");
		return 0;
	}
	unsigned long len = lseek(fh, 0L, SEEK_END);
	lseek(fh, 0L, SEEK_SET);
	char *input = MALLOC_N(len+1, char);
	len = read(fh, input, len);
	input[len] = '\0';

	text_buffer_t text_buffer;
	text_buffer_assign_string(&text_buffer, input);
//	text_buffer_assign_string(&text_buffer, "if (a) a = 3;");
	
	solutions_t solutions;
	solutions_init(&solutions, &text_buffer);

	init_expected();
	
	parser_t parser;
	parser_init(&parser, &text_buffer);
	parser.cache_hit_function = solutions_find;
	parser.cache = &solutions;
	
	ENTER_RESULT_CONTEXT

	DECL_RESULT(result);
	if (!parse_nt(&parser, find_nt("root", &all_nt), &result) || !text_buffer_end(&text_buffer))
	{
		print_expected(stdout);
	}
	else
	{
		if (result.data == NULL)
			fprintf(stderr, "ERROR: parsing did not return result\n");
		else
		{
			char output[200];
			fixed_string_ostream_t fixed_string_ostream;
			fixed_string_ostream_init(&fixed_string_ostream, output, 200);
			result_print(&result, &fixed_string_ostream.ostream);
			fixed_string_ostream_finish(&fixed_string_ostream);
		}
	}
	print((tree_node_p)result.data);
	DISP_RESULT(result);
	
	solutions_free(&solutions);

	EXIT_RESULT_CONTEXT
	
	printf("%d\n", longest_string);

	return 0;
}

