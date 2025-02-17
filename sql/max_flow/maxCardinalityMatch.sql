/*PGR-GNU*****************************************************************
FILE: maxCardinalityMatch.sql

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Copyright (c) 2016 Andrea Nardelli
mail: nrd.nardelli@gmail.com

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

--v3.4
CREATE FUNCTION pgr_maxCardinalityMatch(
  TEXT, -- edges_sql (required)

  OUT edge BIGINT)
RETURNS SETOF BIGINT AS
$BODY$
SELECT edge
FROM _pgr_maxCardinalityMatch(_pgr_get_statement($1), false)
$BODY$
LANGUAGE SQL VOLATILE STRICT
COST 100
ROWS 1000;

--v3.4
CREATE FUNCTION pgr_maxCardinalityMatch(
  TEXT, -- edges_sql (required)

  directed BOOLEAN,

  OUT seq INTEGER,
  OUT edge BIGINT,
  OUT source BIGINT,
  OUT target BIGINT)
RETURNS SETOF RECORD AS
$BODY$
BEGIN
RAISE WARNING 'pgr_maxCardinalityMatch(text,boolean) deprecated signature on v3.4.0';
RETURN QUERY SELECT a.seq, a.edge, a.source, a.target
FROM _pgr_maxCardinalityMatch(_pgr_get_statement($1), $2) AS a;
END
$BODY$
LANGUAGE plpgsql VOLATILE STRICT
COST 100
ROWS 1000;

-- COMMENTS
COMMENT ON FUNCTION pgr_maxCardinalityMatch(TEXT, BOOLEAN)
IS 'pgr_maxCardinalityMatch
- DEPRECATED signature on v3.4.0
- Use without boolean paramater
- Regardless of boolen value it will work for undirected graphs
- Documentation:
  - ${PROJECT_DOC_LINK}/pgr_maxCardinalityMatch.html
';

COMMENT ON FUNCTION pgr_maxCardinalityMatch(TEXT)
IS 'pgr_maxCardinalityMatch
- Parameters:
  - Edges SQL with columns: id, source, target, cost [,reverse_cost]
- Documentation:
- ${PROJECT_DOC_LINK}/pgr_maxCardinalityMatch.html
';
