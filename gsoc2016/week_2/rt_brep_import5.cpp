int
rt_brep_import5(struct rt_db_internal *ip, const struct bu_external *ep, const fastf_t *mat, const struct db_i *dbip)
{
    unsigned char *cp;
    size_t rem;
    size_t thickness_data_block_size = 0;
    size_t brep_data_block_size;
    unsigned char plate_mode_flag;

    ON::Begin();
    TRACE1("rt_brep_import5");

    struct rt_brep_internal* bi;
    if (dbip) RT_CK_DBI(dbip);
    BU_CK_EXTERNAL(ep);
    RT_CK_DB_INTERNAL(ip);
    ip->idb_major_type = DB5_MAJORTYPE_BRLCAD;
    ip->idb_type = ID_BREP;
    ip->idb_meth = &OBJ[ID_BREP];
    BU_ALLOC(ip->idb_ptr, struct rt_brep_internal);

    bi = (struct rt_brep_internal*)ip->idb_ptr;
    bi->magic = RT_BREP_INTERNAL_MAGIC;

    cp = ep->ext_buf;
    rem = ep->ext_nbytes;

    plate_mode_flag = *cp++;
    rem -= 1;

    if (plate_mode_flag == '1')
	thickness_data_block_size = SIZEOF_NETWORK_DOUBLE;
    brep_data_block_size = rem - thickness_data_block_size;

    //RT_MemoryArchive archive(ep->ext_buf, ep->ext_nbytes);
    RT_MemoryArchive archive(cp, brep_data_block_size);
    cp += brep_data_block_size;
    rem -= brep_data_block_size;

    ONX_Model model;
    ON_TextLog dump(stderr);
    //archive.Dump3dmChunk(dump);
    model.Read(archive, &dump);

    //if (model.IsValid(&dump)) {
    ONX_Model_Object mo = model.m_object_table[0];
    // XXX does openNURBS force us to copy? it seems the answer is
    // YES due to the const-ness
    bi->brep = ON_Brep::New(*ON_Brep::Cast(mo.m_object));

    double scan;
    ON_U u;
    u.d = 0.0;

    if (thickness_data_block_size != 0) {
	bu_cv_ntohd((unsigned char*)&scan, cp, 1);
	cp += SIZEOF_NETWORK_DOUBLE;
	rem -= SIZEOF_NETWORK_DOUBLE;
	
	u.d = scan;
    }

    bi->brep->m_brep_user = u;
    for (int fi = 0; fi < bi->brep->m_F.Count(); fi++)
	bi->brep->m_F[fi].m_face_user = u;

    if (mat) {
	ON_Xform xform(mat);

	if (!xform.IsIdentity()) {
	    bi->brep->Transform(xform);
	}
    }
    return 0;
    //} else {
    //	return -1;
    //}
}