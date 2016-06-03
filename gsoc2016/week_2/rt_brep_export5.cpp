int
rt_brep_export5(struct bu_external *ep, const struct rt_db_internal *ip, double UNUSED(local2mm), const struct db_i *dbip)
{
    TRACE1("rt_brep_export5");
    struct rt_brep_internal* bi;

    if (dbip) RT_CK_DBI(dbip);
    RT_CK_DB_INTERNAL(ip);
    if (ip->idb_type != ID_BREP) return -1;
    bi = (struct rt_brep_internal*)ip->idb_ptr;
    RT_BREP_CK_MAGIC(bi);

    BU_EXTERNAL_INIT(ep);

    RT_MemoryArchive archive;
    /* XXX what to do about the version */
    ONX_Model model;

    {
	ON_Layer default_layer;
	default_layer.SetLayerIndex(0);
	default_layer.SetLayerName("Default");
	model.m_layer_table.Reserve(1);
	model.m_layer_table.Append(default_layer);
    }

    ONX_Model_Object& mo = model.m_object_table.AppendNew();
    mo.m_object = bi->brep;
    mo.m_attributes.m_layer_index = 0;
    mo.m_attributes.m_name = "brep";
    mo.m_attributes.m_uuid = ON_opennurbs4_id;

    model.m_properties.m_RevisionHistory.NewRevision();
    model.m_properties.m_Application.m_application_name = "BRL-CAD B-Rep primitive";

    model.Polish();
    ON_TextLog err(stderr);
    bool ok = model.Write(archive, 4, "export5", &err);
    if (ok) {
	unsigned char* cp;
	size_t rem;

	ep->ext_nbytes = (long)archive.Size();
	ep->ext_nbytes += 1; /* plate mode flag */

	if (!NEAR_ZERO(bi->brep->m_brep_user.d, 0.001))
	    ep->ext_nbytes += SIZEOF_NETWORK_DOUBLE;
	ep->ext_buf = (uint8_t *)bu_malloc(ep->ext_nbytes, "B-Rep external");

	cp = ep->ext_buf;
	rem = ep->ext_nbytes;

	if (!NEAR_ZERO(bi->brep->m_brep_user.d, 0.001))
	    *cp++ = '1';
	else
	    *cp++ = '0';
	rem -= 1;	

	/* save archive */
	memcpy(cp, archive.CreateCopy(), (long)archive.Size());
	cp += (long)archive.Size();
	rem -= (long)archive.Size();

	if (!NEAR_ZERO(bi->brep->m_brep_user.d, 0.001)) {
	    double tmp;
	    tmp = bi->brep->m_brep_user.d; /* or * local2mm; */
	    bu_cv_htond(cp, (const unsigned char *)&tmp, 1);
	    cp += SIZEOF_NETWORK_DOUBLE;
	    rem -= SIZEOF_NETWORK_DOUBLE;
	}

	return 0;
    } else {
	return -1;
    }
}