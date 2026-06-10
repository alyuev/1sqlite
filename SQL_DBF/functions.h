// functions.h
#pragma once

#include "StdAfx.h"
#include <math.h>
// Этот файл содержит дополнительные функции

struct func
{
    static void fn_sqrt(sqlite3_context *context, int argc, sqlite3_value ** argv);
    static void rms_step(sqlite3_context *context, int argc, sqlite3_value **argv);
    static void rms_fin(sqlite3_context *context);

};
