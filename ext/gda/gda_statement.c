#include <gda.h>

VALUE cStatement;
VALUE cStructure;

static VALUE serialize(VALUE self)
{
    GdaStatement * stmt;
    gchar * string;

    Data_Get_Struct(self, GdaStatement, stmt);

    string = gda_statement_serialize(stmt);
    return rb_str_new2(string);
}

static VALUE ast(VALUE self)
{
    GdaSqlStatement * sqlst;

    Data_Get_Struct(self, GdaSqlStatement, sqlst);

    return WrapAnyPart(self, GDA_SQL_ANY_PART(sqlst->contents));
}

static VALUE sql(VALUE self)
{
    GdaSqlStatement * sqlst;

    Data_Get_Struct(self, GdaSqlStatement, sqlst);

    return rb_str_new2(sqlst->sql);
}

static VALUE structure(VALUE self)
{
    GdaStatement * stmt;
    GdaSqlStatement * sqlst;

    Data_Get_Struct(self, GdaStatement, stmt);

    g_object_get(G_OBJECT(stmt), "structure", &sqlst, NULL);

    return Data_Wrap_Struct(cStructure, NULL, gda_sql_statement_free, sqlst);
}

static VALUE render(VALUE self)
{
    VALUE return_value = Qnil;

    GdaSqlStatement *sqlstmt = NULL;
    Data_Get_Struct(self, GdaSqlStatement, sqlstmt);
    if(sqlstmt)
    {
        GdaStatement *stmt = gda_statement_new();
        g_object_set(G_OBJECT(stmt), "structure", sqlstmt, NULL);

        GError *error = NULL;
        gchar *sql = gda_statement_to_sql(stmt, NULL, &error);

        if(error)
        {
            g_error_free(error);
            // TODO: rescue
        }
        else
        {
            return_value = rb_str_new2(sql);
            g_free(sql);
        }

        g_object_unref(stmt);
    }

    return return_value;
}

void Init_gda_statement()
{
    cStatement = rb_define_class_under(mSQL, "Statement", rb_cObject);
    cStructure = rb_define_class_under(mSQL, "Structure", rb_cObject);

    rb_define_method(cStatement, "serialize", serialize, 0);
    rb_define_method(cStatement, "structure", structure, 0);
    rb_define_method(cStructure, "ast", ast, 0);
    rb_define_method(cStructure, "sql", sql, 0);
    rb_define_method(cStructure, "render", render, 0);
}

/* vim: set noet sws=4 sw=4: */
