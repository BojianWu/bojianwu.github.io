/** @file proc-db/brep-brep.cpp
 * 
 * Transforming an old brep representation into new format, specifically,
 * integrating face thickness information and turning it into plate mode.
 */

#include "common.h"

#ifdef OBJ_BREP

#ifdef __cplusplus
extern "C" {
#endif

#include "wdb.h"

#ifdef __cplusplus
};
#endif

static void
usage(const char *argv0)
{
    bu_log("Usage: %s [options] src_brep dst_brep\n\n", argv0);

    bu_log("    -H plate_thickness\n"
         "\t\tThickness(mm) used when a NURBS is not a closed volume and it's\n"
         "\t\tconverted as a plate mode NURBS\n");

    bu_log("    src_brep\n"
         "\t\tOld brep representation that does not have plate mode flag\n");

    bu_log("    dst_brep\n"
         "\t\tNew brep representation that integrates plate mode flag\n");
}

int
main(int argc, char *argv[])
{
    ON_Brep* brep = NULL;

    fastf_t thickness = 0.0;
    char* src;
    char* dst;

    if (argc > 1) {
	if (BU_STR_EQUAL(argv[1], "-H")) {
	    thickness = (fastf_t)atof(argv[2]);
	    src = argv[3];
	    dst = argv[4];
	} else {
	    src = argv[1];
	    dst = argv[2];
	}
    } else {
	usage(argv[0]);
	bu_exit(1, "ERROR: please provide enough arguments");
    }

    /* Open BRL-CAD database */
    struct db_i *dbip;
    struct directory* dp;

    if ((dbip = db_open(src, DB_OPEN_READONLY)) == DBI_NULL) {
	bu_exit(1, "Unable to open geometry database file (%s)\n", src);
    }
    RT_CK_DBI(dbip);
    if (db_dirbuild(dbip)) {
	bu_exit(1, "db_dirbuild failed\n");
    }

    FOR_ALL_DIRECTORY_START(dp, dbip) {
	struct rt_db_internal ip;
	struct rt_brep_internal* bi;

	/* Ignore the _GLOBAL object */
	if (dp->d_major_type == DB5_MAJORTYPE_ATTRIBUTE_ONLY && dp->d_minor_type == 0)
	    continue;

	if (rt_db_get_internal(&ip, dp, dbip, NULL, &rt_uniresource) < 0) {
	    bu_log("Unable to read '%s', skipping\n", dp->d_namep);
	    continue;
	}

	if (!ip.idb_meth->ft_get) {
	    bu_log("Unable to get '%s' (unimplemented), skipping\n", dp->d_namep);
	    continue;
	}

	/* Consider only solid object here */
	if (dp->d_flags & RT_DIR_SOLID) {
	    bi = (struct rt_brep_internal*)ip.idb_ptr;
	    bi->magic = RT_BREP_INTERNAL_MAGIC;
	    brep = new ON_Brep(*bi->brep);
	}

	rt_db_free_internal(&ip);
    } FOR_ALL_DIRECTORY_END


    /* Output */
    if (brep) {
	struct rt_wdb* dst_fp;
	const char* dst_geom_name = "surf.s";
	ON_U u;

	u.d = thickness;
	brep->m_brep_user = u;

	ON::Begin();

	bu_log("Writing a b-rep surface with new representation...\n");
	dst_fp = wdb_fopen(dst);
	mk_brep(dst_fp, dst_geom_name, brep);

	unsigned char rgb[] = {50, 255, 50};
	mk_region1(dst_fp, "surf.r", dst_geom_name, "plastic", "", rgb);
	wdb_close(dst_fp);

	ON::End();

	delete brep;
    }

    return 0;
}

#else /* !OBJ_BREP */

int
main(int argc, char *argv[])
{
    bu_log("ERROR: Boundary representation object support is not available with\n"
           "       this compilation of BRL-CAD.\n");
    return 1;
}

#endif /* OBJ_BREP */

/*
 * Local Variables:
 * tab-width: 8
 * mode: C++
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */