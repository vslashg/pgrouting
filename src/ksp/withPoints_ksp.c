/*PGR-GNU*****************************************************************
File: withPoints_ksp.c

Generated with Template by:
Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Function's developer:
Copyright (c) 2015 Celia Virginia Vergara Castillo
Mail:

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/

#include <stdbool.h>
#include "c_common/postgres_connection.h"

#include "c_types/path_rt.h"
#include "c_common/time_msg.h"
#include "c_common/e_report.h"

#include "c_common/pgdata_getters.h"

#include "drivers/withPoints/get_new_queries.h"
#include "drivers/yen/withPoints_ksp_driver.h"
#include "c_common/debug_macro.h"

PGDLLEXPORT Datum _pgr_withpointsksp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(_pgr_withpointsksp);


static
void
process(
        char* edges_sql,
        char* points_sql,
        int64_t start_pid,
        int64_t end_pid,
        int p_k,

        bool directed,
        bool heap_paths,
        char *driving_side,
        bool details,

        Path_rt **result_tuples,
        size_t *result_count) {
    if (p_k < 0) {
        return;
    }

    size_t k = (size_t)p_k;

    driving_side[0] = (char) tolower(driving_side[0]);
    PGR_DBG("driving side:%c", driving_side[0]);
    if (!((driving_side[0] == 'r')
                || (driving_side[0] == 'l'))) {
        driving_side[0] = 'b';
    }

    pgr_SPI_connect();

    Point_on_edge_t *points = NULL;
    size_t total_points = 0;
    pgr_get_points(points_sql, &points, &total_points);

    char *edges_of_points_query = NULL;
    char *edges_no_points_query = NULL;
    get_new_queries(
            edges_sql, points_sql,
            &edges_of_points_query,
            &edges_no_points_query);


    Edge_t *edges_of_points = NULL;
    size_t total_edges_of_points = 0;
    pgr_get_edges(edges_of_points_query, &edges_of_points, &total_edges_of_points, true, false);

    Edge_t *edges = NULL;
    size_t total_edges = 0;
    pgr_get_edges(edges_no_points_query, &edges, &total_edges, true, false);

    PGR_DBG("freeing allocated memory not used anymore");
    pfree(edges_of_points_query);
    pfree(edges_no_points_query);

    if ((total_edges + total_edges_of_points) == 0) {
        PGR_DBG("No edges found");
        (*result_count) = 0;
        (*result_tuples) = NULL;
        pgr_SPI_finish();
        return;
    }

    PGR_DBG("Starting processing");
    clock_t start_t = clock();

    char *log_msg = NULL;
    char *notice_msg = NULL;
    char *err_msg = NULL;
    do_pgr_withPointsKsp(
            edges,
            total_edges,
            points,
            total_points,
            edges_of_points,
            total_edges_of_points,
            start_pid,
            end_pid,
            k,

            directed,
            heap_paths,
            driving_side[0],
            details,

            result_tuples,
            result_count,

            &log_msg,
            &notice_msg,
            &err_msg);
    time_msg(" processing withPointsKSP", start_t, clock());

    if (err_msg && (*result_tuples)) {
        pfree(*result_tuples);
        (*result_tuples) = NULL;
        (*result_count) = 0;
    }

    pgr_global_report(log_msg, notice_msg, err_msg);

    if (log_msg) pfree(log_msg);
    if (notice_msg) pfree(notice_msg);
    if (err_msg) pfree(err_msg);

    pfree(edges);
    pfree(edges_of_points);
    pfree(points);

    pgr_SPI_finish();
}




PGDLLEXPORT Datum _pgr_withpointsksp(PG_FUNCTION_ARGS) {
    FuncCallContext     *funcctx;
    TupleDesc            tuple_desc;

    Path_rt  *result_tuples = 0;
    size_t result_count = 0;

    if (SRF_IS_FIRSTCALL()) {
        MemoryContext   oldcontext;
        funcctx = SRF_FIRSTCALL_INIT();
        oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);


        /*
           CREATE OR REPLACE FUNCTION pgr_withPoint(
           edges_sql TEXT,
           points_sql TEXT,
           start_pid INTEGER,
           end_pid BIGINT,
           k BIGINT,

           directed BOOLEAN -- DEFAULT true,
           heap_paths BOOLEAN -- DEFAULT false,
           driving_side CHAR -- DEFAULT 'b',
           details BOOLEAN -- DEFAULT false
        */

        PGR_DBG("Calling process");
        PGR_DBG("initial driving side:%s",
                text_to_cstring(PG_GETARG_TEXT_P(7)));
        process(
                text_to_cstring(PG_GETARG_TEXT_P(0)),
                text_to_cstring(PG_GETARG_TEXT_P(1)),
                PG_GETARG_INT64(2),
                PG_GETARG_INT64(3),
                PG_GETARG_INT32(4),
                PG_GETARG_BOOL(5),
                PG_GETARG_BOOL(6),
                text_to_cstring(PG_GETARG_TEXT_P(7)),
                PG_GETARG_BOOL(8),
                &result_tuples,
                &result_count);


        funcctx->max_calls = result_count;

        funcctx->user_fctx = result_tuples;
        if (get_call_result_type(fcinfo, NULL, &tuple_desc)
                != TYPEFUNC_COMPOSITE)
            ereport(ERROR,
                    (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                     errmsg("function returning record called in context "
                         "that cannot accept type record")));

        funcctx->tuple_desc = tuple_desc;
        MemoryContextSwitchTo(oldcontext);
    }

    funcctx = SRF_PERCALL_SETUP();
    tuple_desc = funcctx->tuple_desc;
    result_tuples = (Path_rt*) funcctx->user_fctx;

    if (funcctx->call_cntr < funcctx->max_calls) {
        HeapTuple    tuple;
        Datum        result;
        Datum        *values;
        bool*        nulls;

        values = palloc(7 * sizeof(Datum));
        nulls = palloc(7 * sizeof(bool));

        size_t i;
        for (i = 0; i < 7; ++i) {
            nulls[i] = false;
        }

        /*
           OUT seq INTEGER, OUT path_id INTEGER, OUT path_seq INTEGER,
           OUT node BIGINT, OUT edge BIGINT,
           OUT cost FLOAT, OUT agg_cost FLOAT)
           */


        // postgres starts counting from 1
        values[0] = Int32GetDatum(funcctx->call_cntr + 1);
        values[1] = Int32GetDatum((int)
                (result_tuples[funcctx->call_cntr].start_id + 1));
        values[2] = Int32GetDatum(result_tuples[funcctx->call_cntr].seq);
        values[3] = Int64GetDatum(result_tuples[funcctx->call_cntr].node);
        values[4] = Int64GetDatum(result_tuples[funcctx->call_cntr].edge);
        values[5] = Float8GetDatum(result_tuples[funcctx->call_cntr].cost);
        values[6] = Float8GetDatum(result_tuples[funcctx->call_cntr].agg_cost);

        tuple = heap_form_tuple(tuple_desc, values, nulls);
        result = HeapTupleGetDatum(tuple);
        SRF_RETURN_NEXT(funcctx, result);
    } else {
        SRF_RETURN_DONE(funcctx);
    }
}

