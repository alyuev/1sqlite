
#include <functions.h>

void func::fn_sqrt(sqlite3_context *context, int argc, sqlite3_value ** argv) {
    double  val;

    int type = sqlite3_value_type(argv[0]);

    if((type == SQLITE_INTEGER) || (type == SQLITE_FLOAT)) {
        val = sqlite3_value_double(argv[0]);
        if(val >= 0) {
            val = sqrt(val);
            if((long)val == val) {
                sqlite3_result_int64(context, (long)val);
            } else {
                sqlite3_result_double(context, val);
            }
        } else {
            sqlite3_result_null(context);
        }
    } else {
        sqlite3_result_int(context, 0);
    }
}

typedef struct SumCtx SumCtx;
struct SumCtx {
    double sum;
    int cnt;
};

void func::rms_step(sqlite3_context *context, int argc, sqlite3_value **argv) {

    int type;
    double val;

    SumCtx *p = (SumCtx*) sqlite3_aggregate_context(context, sizeof(*p));

    type = sqlite3_value_numeric_type(argv[0]);

    if((type == SQLITE_INTEGER) || (type == SQLITE_FLOAT)) {
        val = sqlite3_value_double(argv[0]);
        p->cnt++;
        p->sum += val * val;
    }
}

void func::rms_fin(sqlite3_context *context) {
    double val;

    SumCtx *p = (SumCtx*) sqlite3_aggregate_context(context, sizeof(*p));

    if(p && p->cnt > 0) {
        val = sqrt(p->sum/p->cnt);
        if((long)val == val) {
            sqlite3_result_int64(context, (long)val);
        } else {
            sqlite3_result_double(context, val);
        }
    }
    else    {
        sqlite3_result_int(context, 0);
    }
}

