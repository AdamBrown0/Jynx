<program> ::= { <import_decl> | <include_decl> | <class_decl> }

<include_decl> ::= "include" <qualified_name> ";"
<import_decl>  ::= "import" <qualified_name> ";"

<qualified_name> ::= <identifier> { "::" <identifier> }

<class_decl> ::= "class" <identifier> "{" { <class_member> } "}"

<class_member> ::= <field_decl>
                 | <method_decl>
                 | <constructor_decl>

<field_decl> ::= <access_modifier> [ "static" ] <type> <identifier> ";"

<constructor_decl> ::= <identifier> "(" [ <param_list> ] ")" <block>

<method_decl> ::= <access_modifier> [ "static" ] <type> <identifier> "(" [ <param_list> ] ")" <block>

<param_list> ::= <param> { "," <param> }
<param> ::= <type> <identifier>

<block> ::= "{" { <statement> } "}"

<statement> ::= <var_decl_stmt>
              | <expr_stmt>
              | <if_stmt>
              | <return_stmt>
              | <block>
              | ";"

<var_decl_stmt> ::= <type> <identifier> [ "=" <expression> ] ";"

<if_stmt> ::= "if" "(" <expression> ")" <statement> [ "else" <statement> ]

<return_stmt> ::= "return" <expression> ";"

<expr_stmt> ::= <expression> ";"

<expression> ::= <assignment_expr>

<assignment_expr> ::= <logical_or_expr> [ "=" <assignment_expr> ]

<logical_or_expr> ::= <logical_and_expr> { "||" <logical_and_expr> }
<logical_and_expr> ::= <equality_expr> { "&&" <equality_expr> }

<equality_expr> ::= <relational_expr> { ("==" | "!=") <relational_expr> }

<relational_expr> ::= <additive_expr> { ("<" | ">" | "<=" | ">=") <additive_expr> }

<additive_expr> ::= <multiplicative_expr> { ("+" | "-") <multiplicative_expr> }

<multiplicative_expr> ::= <unary_expr> { ("*" | "/") <unary_expr> }

<unary_expr> ::= [ "!" | "-" ] <primary_expr>

<primary_expr> ::= <literal>
                 | <identifier>
                 | <method_call>
                 | "new" <identifier> "(" [ <argument_list> ] ")"
                 | "(" <expression> ")"

<method_call> ::= <expression> "." <identifier> "(" [ <argument_list> ] ")"

<argument_list> ::= <expression> { "," <expression> }

<literal> ::= <int_literal>
            | <string_literal>
            | <multiline_string>
            | "true"
            | "false"
            | "null"

<type> ::= "int"
         | "string"
         | "void"
         | <identifier>    (* for user-defined classes *)

<access_modifier> ::= "public" | "private" | "protected"

