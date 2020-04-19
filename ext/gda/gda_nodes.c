#include <gda.h>

VALUE mNodes;
VALUE cSelect;
VALUE cFrom;
VALUE cSelectField;
VALUE cExpr;
VALUE cOrder;
VALUE cNode;
VALUE cOperation;
VALUE cTarget;
VALUE cFunction;
VALUE cInsert;
VALUE cDelete;
VALUE cUpdate;
VALUE cTable;
VALUE cUnknown;
VALUE cJoin;
VALUE cField;
VALUE cRollbackSavepoint;
VALUE cDeleteSavepoint;
VALUE cSavepoint;
VALUE cBegin;
VALUE cRollback;
VALUE cCommit;
VALUE cCompound;

#define node_deallocate(node, deallocator) \
    if(!GDA_SQL_ANY_PART(node)->parent) \
      deallocator(node);

#define WrapInteger(klass, type, lname) \
    static VALUE rb_##klass##_##lname(VALUE self) \
{ \
    type *st;\
    Data_Get_Struct(self, type, st); \
    return INT2NUM(st->lname); \
}

#define WrapBoolean(klass, type, lname) \
    static VALUE rb_##klass##_##lname(VALUE self) \
{ \
    type *st;\
    Data_Get_Struct(self, type, st); \
    if (st->lname) \
      return Qtrue; \
    else \
      return Qfalse; \
}

#define WrapString(klass, type, lname) \
    static VALUE rb_##klass##_##lname(VALUE self) \
{ \
    type *st;\
    Data_Get_Struct(self, type, st); \
    if (st->lname) \
      return rb_str_new2(st->lname); \
    else \
      return Qnil; \
}

#define WrapNode(klass, type, lname) \
    static VALUE rb_##klass##_##lname(VALUE self) \
{ \
    type *st;\
    VALUE stmt;\
    Data_Get_Struct(self, type, st); \
    stmt = rb_iv_get(self, "stmt"); \
    return WrapAnyPart(stmt, (GdaSqlAnyPart *)st->lname); \
}

#define WrapList(klass, type, lname) \
    static VALUE rb_##klass##_##lname(VALUE self) \
{ \
    type *ptr; \
    GSList *list; \
    VALUE rb_list; \
    VALUE stmt; \
    Data_Get_Struct(self, type, ptr);\
    stmt = rb_iv_get(self, "stmt"); \
    rb_list = rb_ary_new(); \
    list = ptr->lname; \
    while(list) { \
	rb_ary_push(rb_list, WrapAnyPart(stmt, (GdaSqlAnyPart *)list->data)); \
	list = list->next; \
    } \
    return rb_list; \
}

#define WrapperMethod(klass, lname) \
    rb_define_method(klass, #lname, rb_##klass##_##lname, 0);

WrapString(cTable, GdaSqlTable, table_name);

WrapString(cField, GdaSqlField, field_name);

WrapNode(cSelect, GdaSqlStatementSelect, distinct_expr);
WrapList(cSelect, GdaSqlStatementSelect, expr_list);
WrapNode(cSelect, GdaSqlStatementSelect, from);
WrapNode(cSelect, GdaSqlStatementSelect, where_cond);
WrapList(cSelect, GdaSqlStatementSelect, group_by);
WrapNode(cSelect, GdaSqlStatementSelect, having_cond);
WrapList(cSelect, GdaSqlStatementSelect, order_by);
WrapNode(cSelect, GdaSqlStatementSelect, limit_count);
WrapNode(cSelect, GdaSqlStatementSelect, limit_offset);

WrapNode(cSelectField, GdaSqlSelectField, expr);
WrapString(cSelectField, GdaSqlSelectField, field_name);
WrapString(cSelectField, GdaSqlSelectField, table_name);
WrapString(cSelectField, GdaSqlSelectField, as);

WrapNode(cExpr, GdaSqlExpr, func);
WrapNode(cExpr, GdaSqlExpr, cond);
WrapNode(cExpr, GdaSqlExpr, select);
WrapNode(cExpr, GdaSqlExpr, case_s);
WrapNode(cExpr, GdaSqlExpr, param_spec);
WrapString(cExpr, GdaSqlExpr, cast_as);
WrapBoolean(cExpr, GdaSqlExpr, value_is_ident);

WrapList(cFrom, GdaSqlSelectFrom, targets);
WrapList(cFrom, GdaSqlSelectFrom, joins);

WrapList(cOperation, GdaSqlOperation, operands);

WrapNode(cTarget, GdaSqlSelectTarget, expr);
WrapString(cTarget, GdaSqlSelectTarget, table_name);
WrapString(cTarget, GdaSqlSelectTarget, as);

WrapList(cFunction, GdaSqlFunction, args_list);
WrapString(cFunction, GdaSqlFunction, function_name);

WrapNode(cOrder, GdaSqlSelectOrder, expr);
WrapBoolean(cOrder, GdaSqlSelectOrder, asc);
WrapString(cOrder, GdaSqlSelectOrder, collation_name);

WrapNode(cInsert, GdaSqlStatementInsert, table);
WrapList(cInsert, GdaSqlStatementInsert, fields_list);
WrapNode(cInsert, GdaSqlStatementInsert, select);
WrapString(cInsert, GdaSqlStatementInsert, on_conflict);

WrapNode(cUpdate, GdaSqlStatementUpdate, table);
WrapList(cUpdate, GdaSqlStatementUpdate, fields_list);
WrapList(cUpdate, GdaSqlStatementUpdate, expr_list);
WrapNode(cUpdate, GdaSqlStatementUpdate, cond);
WrapString(cUpdate, GdaSqlStatementUpdate, on_conflict);

WrapNode(cDelete, GdaSqlStatementDelete, table);
WrapNode(cDelete, GdaSqlStatementDelete, cond);

WrapNode(cJoin, GdaSqlSelectJoin, expr);
WrapList(cJoin, GdaSqlSelectJoin, use);
WrapInteger(cJoin, GdaSqlSelectJoin, position);

WrapInteger(cCompound, GdaSqlStatementCompound, compound_type);

WrapList(cUnknown, GdaSqlStatementUnknown, expressions);

static VALUE distinct_p(VALUE self)
{
    GdaSqlStatementSelect * st;

    Data_Get_Struct(self, GdaSqlStatementSelect, st);

    if (st->distinct)
	return Qtrue;

    return Qfalse;
}

static VALUE Wrap(VALUE stmt, VALUE klass, GdaSqlAnyPart *part)
{
    VALUE obj = Data_Wrap_Struct(klass, NULL, NULL, part);
    rb_iv_set(obj, "stmt", stmt);
    return obj;
}

VALUE WrapAnyPart(VALUE stmt, GdaSqlAnyPart *part)
{
    if (!part)
	return Qnil;

    switch(part->type) {
	case GDA_SQL_ANY_STMT_SELECT:
	    return Wrap(stmt, cSelect, part);
	    break;
	case GDA_SQL_ANY_STMT_INSERT:
	    return Wrap(stmt, cInsert, part);
	    break;
	case GDA_SQL_ANY_STMT_UPDATE:
	    return Wrap(stmt, cUpdate, part);
	    break;
	case GDA_SQL_ANY_STMT_DELETE:
	    return Wrap(stmt, cDelete, part);
	    break;
	case GDA_SQL_ANY_STMT_BEGIN:
	    return Wrap(stmt, cBegin, part);
	    break;
	case GDA_SQL_ANY_STMT_ROLLBACK:
	    return Wrap(stmt, cRollback, part);
	    break;
	case GDA_SQL_ANY_STMT_COMMIT:
	    return Wrap(stmt, cCommit, part);
	    break;
	case GDA_SQL_ANY_STMT_SAVEPOINT:
	    return Wrap(stmt, cSavepoint, part);
	    break;
	case GDA_SQL_ANY_STMT_ROLLBACK_SAVEPOINT:
	    return Wrap(stmt, cRollbackSavepoint, part);
	    break;
	case GDA_SQL_ANY_STMT_DELETE_SAVEPOINT:
	    return Wrap(stmt, cDeleteSavepoint, part);
	    break;
	case GDA_SQL_ANY_STMT_UNKNOWN:
	    return Wrap(stmt, cUnknown, part);
	    break;
	case GDA_SQL_ANY_SQL_SELECT_FROM:
	    return Wrap(stmt, cFrom, part);
	    break;
	case GDA_SQL_ANY_SQL_SELECT_FIELD:
	    return Wrap(stmt, cSelectField, part);
	    break;
	case GDA_SQL_ANY_EXPR:
	    return Wrap(stmt, cExpr, part);
	    break;
	case GDA_SQL_ANY_SQL_FIELD:
	    return Wrap(stmt, cField, part);
	    break;
	case GDA_SQL_ANY_SQL_SELECT_ORDER:
	    return Wrap(stmt, cOrder, part);
	    break;
	case GDA_SQL_ANY_SQL_TABLE:
	    return Wrap(stmt, cTable, part);
	    break;
	case GDA_SQL_ANY_SQL_OPERATION:
	    return Wrap(stmt, cOperation, part);
	    break;
	case GDA_SQL_ANY_SQL_FUNCTION:
	    return Wrap(stmt, cFunction, part);
	    break;
	case GDA_SQL_ANY_SQL_SELECT_TARGET:
	    return Wrap(stmt, cTarget, part);
	    break;
	case GDA_SQL_ANY_SQL_SELECT_JOIN:
	    return Wrap(stmt, cJoin, part);
	    break;
    case GDA_SQL_ANY_STMT_COMPOUND:
	    return Wrap(stmt, cCompound, part);
	    break;
	default:
	    rb_raise(rb_eRuntimeError, "unknown node type: %d\n", part->type);
	    return Qnil;
    }
}


static VALUE rb_cCompound_stmt_list(VALUE self)
{
    GdaSqlStatementCompound * st;
    GSList * list;
    VALUE array;
    VALUE stmt;

    Data_Get_Struct(self, GdaSqlStatementCompound, st);
    stmt = rb_iv_get(self, "stmt");
    array = rb_ary_new();

    for (list = st->stmt_list; list; list = list->next) {
      GdaSqlStatement *sqlst = (GdaSqlStatement*) list->data;
      VALUE obj = Wrap(stmt, cSelect, GDA_SQL_ANY_PART(sqlst->contents));
      rb_ary_push(array, obj);
    }
    return array;
}


static VALUE rb_cInsert_values_list(VALUE self)
{
    GdaSqlStatementInsert * st;
    GSList * list;
    VALUE array;
    VALUE stmt;

    Data_Get_Struct(self, GdaSqlStatementInsert, st);

    stmt = rb_iv_get(self, "stmt");

    array = rb_ary_new();
    list = st->values_list;
    while(list) {
	VALUE iarray = rb_ary_new();
	GSList * inner = (GSList *)list->data;

	while(inner) {
	  rb_ary_push(iarray, WrapAnyPart(stmt, (GdaSqlAnyPart *)inner->data));
	  inner = inner->next;
	}

	rb_ary_push(array, iarray);
	list = list->next;
    }

    return array;
}

static VALUE rb_cJoin_join_type(VALUE self)
{
    GdaSqlSelectJoin * st;

    Data_Get_Struct(self, GdaSqlSelectJoin, st);

    if (st->type)
	return rb_str_new2(gda_sql_select_join_type_to_string(st->type));

    return Qnil;
}

static VALUE rb_st_type(VALUE self)
{
    GdaSqlAnyPart * st;

    Data_Get_Struct(self, GdaSqlAnyPart, st);

    switch(st->type) {
	case GDA_SQL_ANY_STMT_BEGIN:
	    return rb_str_new2("BEGIN");
	    break;
	case GDA_SQL_ANY_STMT_ROLLBACK:
	    return rb_str_new2("ROLLBACK");
	    break;
	case GDA_SQL_ANY_STMT_COMMIT:
	    return rb_str_new2("COMMIT");
	    break;
	case GDA_SQL_ANY_STMT_SAVEPOINT:
	    return rb_str_new2("SAVEPOINT");
	    break;
	case GDA_SQL_ANY_STMT_ROLLBACK_SAVEPOINT:
	    return rb_str_new2("ROLLBACK SAVEPOINT");
	    break;
	case GDA_SQL_ANY_STMT_DELETE_SAVEPOINT:
	    return rb_str_new2("DELETE SAVEPOINT");
	    break;
	default:
	    return Qnil;
    }
}

static VALUE rb_st_isolation_level(VALUE self)
{
    GdaSqlStatementTransaction * st;

    Data_Get_Struct(self, GdaSqlStatementTransaction, st);

    switch(st->isolation_level) {
	default:
	case GDA_TRANSACTION_ISOLATION_UNKNOWN:
	    return rb_str_new2("UNKNOWN");
	    break;
	case GDA_TRANSACTION_ISOLATION_READ_COMMITTED:
	    return rb_str_new2("READ COMMITTED");
	    break;
	case GDA_TRANSACTION_ISOLATION_READ_UNCOMMITTED:
	    return rb_str_new2("READ UNCOMMITTED");
	    break;
	case GDA_TRANSACTION_ISOLATION_REPEATABLE_READ:
	    return rb_str_new2("REPEATABLE READ");
	    break;
	case GDA_TRANSACTION_ISOLATION_SERIALIZABLE:
	    return rb_str_new2("SERIALIZABLE");
	    break;
    }
}

static VALUE rb_st_trans_mode(VALUE self)
{
    GdaSqlStatementTransaction * st;

    Data_Get_Struct(self, GdaSqlStatementTransaction, st);

    if (st->trans_mode)
	return rb_str_new2(st->trans_mode);

    return Qnil;
}

static VALUE rb_st_trans_name(VALUE self)
{
    GdaSqlStatementTransaction * st;

    Data_Get_Struct(self, GdaSqlStatementTransaction, st);

    if (st->trans_name)
	return rb_str_new2(st->trans_name);

    return Qnil;
}

static void rb_cExpr_deallocate(GdaSqlExpr *expr)
{
    node_deallocate(expr, gda_sql_expr_free);
}

static VALUE rb_cExpr_allocate(VALUE self)
{
    return Data_Wrap_Struct(cExpr, NULL, rb_cExpr_deallocate, gda_sql_expr_new(NULL));
}

static VALUE rb_cExpr_value(VALUE self)
{
    GdaSqlExpr * st;
    GValue * val;

    Data_Get_Struct(self, GdaSqlExpr, st);

    val = st->value;

    return rb_str_new2(gda_value_stringify(val));
}

static VALUE rb_cExpr_set_value(VALUE self, VALUE value)
{
    GdaSqlExpr *expr = NULL;
    Data_Get_Struct(self, GdaSqlExpr, expr);

    if(!expr->value)
        expr->value = g_new0 (GValue, 1);

    g_value_init(expr->value, G_TYPE_STRING);
    g_value_set_string(expr->value, StringValueCStr(value));

    return Qnil;
}

static VALUE rb_cExpr_set_cond(VALUE self, VALUE cond)
{
    GdaSqlExpr *expr = NULL;
    Data_Get_Struct(self, GdaSqlExpr, expr);

    GdaSqlOperation *operation = NULL;

    if(!NIL_P(cond))
        Data_Get_Struct(cond, GdaSqlOperation, operation);

    gda_sql_operation_free(expr->cond);
    expr->cond = operation;

    return Qnil;
}

static VALUE rb_cExpr_set_func(VALUE self, VALUE func)
{
    GdaSqlExpr *expr = NULL;
    Data_Get_Struct(self, GdaSqlExpr, expr);
    GdaSqlFunction *function = NULL;

    if(!NIL_P(func))
    {
        Data_Get_Struct(func, GdaSqlFunction, function);
        gda_sql_any_part_set_parent(function, expr);
    }

    expr->func = function;

    return Qnil;
}

static void function_deallocate(GdaSqlFunction *function)
{
    node_deallocate(function, gda_sql_function_free);
}

static VALUE rb_cFunction_allocate(VALUE self)
{
    return Data_Wrap_Struct(cFunction, NULL, function_deallocate, gda_sql_function_new(NULL));
}

static VALUE rb_cFunction_set_function_name(VALUE self, VALUE function_name)
{
    GdaSqlFunction *function = NULL;
    Data_Get_Struct(self, GdaSqlFunction, function);

    g_free(function->function_name);
    function->function_name = g_strdup(StringValueCStr(function_name));

    return Qnil;
}

static VALUE rb_cFunction_set_args_list(VALUE self, VALUE args_list)
{
    GdaSqlFunction *function = NULL;
    Data_Get_Struct(self, GdaSqlFunction, function);

    if(function->args_list)
        g_slist_free_full(g_steal_pointer(&function->args_list), (GDestroyNotify) gda_sql_expr_free);

    for(size_t i = 0; i < RARRAY_LEN(args_list); ++i)
    {
        VALUE entry = rb_ary_entry(args_list, i);
        GdaSqlExpr *expr = NULL;
        Data_Get_Struct(entry, GdaSqlExpr, expr);
        function->args_list = g_slist_append(function->args_list, expr);
        gda_sql_any_part_set_parent(expr, function);
    }

    return Qnil;
}

static void operation_deallocate(GdaSqlOperation *operation)
{
    node_deallocate(operation, gda_sql_operation_free);
}

static VALUE rb_cOperation_allocate(VALUE self)
{
    return Data_Wrap_Struct(cOperation, NULL, gda_sql_operation_free, gda_sql_operation_new(NULL));
}

static VALUE rb_cOperation_set_operands(VALUE self, VALUE operands)
{
    GdaSqlOperation *operation = NULL;
    Data_Get_Struct(self, GdaSqlOperation, operation);

    if(operation->operands)
        g_slist_free_full(g_steal_pointer(&operation->operands), (GDestroyNotify) gda_sql_expr_free);

    for(size_t i = 0; i < RARRAY_LEN(operands); ++i)
    {
        VALUE entry = rb_ary_entry(operands, i);
        GdaSqlExpr *expr = NULL;
        Data_Get_Struct(entry, GdaSqlExpr, expr);
        operation->operands = g_slist_append(operation->operands, expr);
    }

    return Qnil;
}

static VALUE rb_cOperation_set_operator(VALUE self, VALUE operator)
{
    const gchar *string = StringValuePtr(operator);
    GdaSqlOperation *operation = NULL;
    Data_Get_Struct(self, GdaSqlOperation, operation);

    operation->operator_type = gda_sql_operation_operator_from_string(string);

    return Qnil;
}

static VALUE rb_cOperation_get_operator(VALUE self)
{
    VALUE return_value = Qnil;

    GdaSqlOperation *operation = NULL;
    Data_Get_Struct(self, GdaSqlOperation, operation);

    return_value = rb_str_new2(gda_sql_operation_operator_to_string(operation->operator_type));

    return return_value;
}

void Init_gda_nodes()
{
    mNodes = rb_define_module_under(mGDA, "Nodes");

    cNode = rb_define_class_under(mNodes, "Node", rb_cObject);

    cSelect = rb_define_class_under(mNodes, "Select", cNode);
    rb_define_method(cSelect, "distinct?", distinct_p, 0);

    WrapperMethod(cSelect, from);
    WrapperMethod(cSelect, distinct_expr);
    WrapperMethod(cSelect, expr_list);
    WrapperMethod(cSelect, where_cond);
    WrapperMethod(cSelect, group_by);
    WrapperMethod(cSelect, having_cond);
    WrapperMethod(cSelect, order_by);
    WrapperMethod(cSelect, limit_count);
    WrapperMethod(cSelect, limit_offset);

    cSelectField = rb_define_class_under(mNodes, "SelectField", cNode);
    WrapperMethod(cSelectField, expr);
    WrapperMethod(cSelectField, field_name);
    WrapperMethod(cSelectField, table_name);
    WrapperMethod(cSelectField, as);

    cExpr = rb_define_class_under(mNodes, "Expr", cNode);
    rb_define_alloc_func(cExpr, rb_cExpr_allocate);
    WrapperMethod(cExpr, func);
    rb_define_method(cExpr, "func=", rb_cExpr_set_func, 1);
    WrapperMethod(cExpr, cond);
    rb_define_method(cExpr, "cond=", rb_cExpr_set_cond, 1);
    WrapperMethod(cExpr, select);
    WrapperMethod(cExpr, case_s);
    WrapperMethod(cExpr, param_spec);
    WrapperMethod(cExpr, cast_as);
    WrapperMethod(cExpr, value);
    rb_define_method(cExpr, "value=", rb_cExpr_set_value, 1);
    WrapperMethod(cExpr, value_is_ident);

    cFrom = rb_define_class_under(mNodes, "From", cNode);
    WrapperMethod(cFrom, targets);
    WrapperMethod(cFrom, joins);

    cOperation = rb_define_class_under(mNodes, "Operation", cNode);
    WrapperMethod(cOperation, operands);
    rb_define_alloc_func(cOperation, rb_cOperation_allocate);
    rb_define_method(cOperation, "operands=", rb_cOperation_set_operands, 1);
    rb_define_method(cOperation, "operator", rb_cOperation_get_operator, 0);
    rb_define_method(cOperation, "operator=", rb_cOperation_set_operator, 1);

    cTarget = rb_define_class_under(mNodes, "Target", cNode);
    WrapperMethod(cTarget, expr);
    WrapperMethod(cTarget, table_name);
    WrapperMethod(cTarget, as);

    cFunction = rb_define_class_under(mNodes, "Function", cNode);
    rb_define_alloc_func(cFunction, rb_cFunction_allocate);
    WrapperMethod(cFunction, args_list);
    rb_define_method(cFunction, "args_list=", rb_cFunction_set_args_list, 1);
    WrapperMethod(cFunction, function_name);
    rb_define_method(cFunction, "function_name=", rb_cFunction_set_function_name, 1);

    cOrder = rb_define_class_under(mNodes, "Order", cNode);
    WrapperMethod(cOrder, expr);
    WrapperMethod(cOrder, asc);
    WrapperMethod(cOrder, collation_name);

    cInsert = rb_define_class_under(mNodes, "Insert", cNode);
    WrapperMethod(cInsert, table);
    WrapperMethod(cInsert, values_list);
    WrapperMethod(cInsert, fields_list);
    WrapperMethod(cInsert, select);
    WrapperMethod(cInsert, on_conflict);

    cDelete = rb_define_class_under(mNodes, "Delete", cNode);
    WrapperMethod(cDelete, table);
    WrapperMethod(cDelete, cond);

    cUpdate = rb_define_class_under(mNodes, "Update", cNode);
    WrapperMethod(cUpdate, table);
    WrapperMethod(cUpdate, fields_list);
    WrapperMethod(cUpdate, expr_list);
    WrapperMethod(cUpdate, cond);
    WrapperMethod(cUpdate, on_conflict);

    cTable = rb_define_class_under(mNodes, "Table", cNode);
    WrapperMethod(cTable, table_name);

    cUnknown = rb_define_class_under(mNodes, "Unknown", cNode);
    WrapperMethod(cUnknown, expressions);

    cJoin = rb_define_class_under(mNodes, "Join", cNode);
    WrapperMethod(cJoin, expr);
    WrapperMethod(cJoin, use);
    WrapperMethod(cJoin, join_type);
    WrapperMethod(cJoin, position);

    cField = rb_define_class_under(mNodes, "Field", cNode);
    WrapperMethod(cField, field_name);

    cCompound = rb_define_class_under(mNodes, "Compound", cNode);
    WrapperMethod(cCompound, compound_type);
    WrapperMethod(cCompound, stmt_list);

    cBegin = rb_define_class_under(mNodes, "Begin", cNode);
    cRollback = rb_define_class_under(mNodes, "Rollback", cNode);
    cCommit = rb_define_class_under(mNodes, "Commit", cNode);
    cSavepoint = rb_define_class_under(mNodes, "Savepoint", cNode);
    cRollbackSavepoint = rb_define_class_under(mNodes, "RollbackSavepoint", cNode);
    cDeleteSavepoint = rb_define_class_under(mNodes, "DeleteSavepoint", cNode);

    rb_define_method(cRollbackSavepoint, "__type__", rb_st_type, 0);
    rb_define_method(cBegin, "__type__", rb_st_type, 0);
    rb_define_method(cSavepoint, "__type__", rb_st_type, 0);
    rb_define_method(cCommit, "__type__", rb_st_type, 0);
    rb_define_method(cRollback, "__type__", rb_st_type, 0);
    rb_define_method(cDeleteSavepoint, "__type__", rb_st_type, 0);

    rb_define_method(cRollbackSavepoint, "isolation_level", rb_st_isolation_level, 0);
    rb_define_method(cBegin, "isolation_level", rb_st_isolation_level, 0);
    rb_define_method(cSavepoint, "isolation_level", rb_st_isolation_level, 0);
    rb_define_method(cCommit, "isolation_level", rb_st_isolation_level, 0);
    rb_define_method(cRollback, "isolation_level", rb_st_isolation_level, 0);
    rb_define_method(cDeleteSavepoint, "isolation_level", rb_st_isolation_level, 0);

    rb_define_method(cRollbackSavepoint, "trans_mode", rb_st_trans_mode, 0);
    rb_define_method(cBegin, "trans_mode", rb_st_trans_mode, 0);
    rb_define_method(cSavepoint, "trans_mode", rb_st_trans_mode, 0);
    rb_define_method(cCommit, "trans_mode", rb_st_trans_mode, 0);
    rb_define_method(cRollback, "trans_mode", rb_st_trans_mode, 0);
    rb_define_method(cDeleteSavepoint, "trans_mode", rb_st_trans_mode, 0);

    rb_define_method(cRollbackSavepoint, "trans_name", rb_st_trans_name, 0);
    rb_define_method(cBegin, "trans_name", rb_st_trans_name, 0);
    rb_define_method(cSavepoint, "trans_name", rb_st_trans_name, 0);
    rb_define_method(cCommit, "trans_name", rb_st_trans_name, 0);
    rb_define_method(cRollback, "trans_name", rb_st_trans_name, 0);
    rb_define_method(cDeleteSavepoint, "trans_name", rb_st_trans_name, 0);
}

/* vim: set noet sws=4 sw=4: */
