bool refine(Mesh& mesh, Real min_qual) {
  mesh.set_partition(GHOSTED);
  auto comm = mesh.comm();
  auto edges_are_cands = mesh.get_array<I8>(EDGE, "candidate");
  auto cands2edges = collect_marked(edges_are_cands);
  auto cand_quals = refine_qualities(mesh, cands2edges);
  auto cands_are_good = each_geq_to(cand_quals, min_qual);
  if (comm->allreduce(max(cands_are_good), MAX) == 0) return false;
  auto nedges = mesh.nents(EDGE);
  auto edges_are_initial_w = Write<I8>(nedges, 0);
  map_into(cands_are_good, cands2edges, edges_are_initial_w, 1);
  auto edges_are_initial = Read<I8>(edges_are_initial_w);
  auto edge_quals_w = Write<Real>(nedges, 0.);
  map_into(cand_quals, cands2edges, edge_quals_w, 1);
  auto edge_quals = Reals(edge_quals_w);
  auto edges_are_keys = find_indset(mesh, EDGE, edge_quals, edges_are_initial);
  mesh.add_tag(EDGE, "edge2rep_order", 1, OSH_DONT_TRANSFER,
      get_edge2rep_order(mesh, edges_are_keys));
  //TODO: alter partition to make cavities local
  auto keys2edges = collect_marked(edges_are_keys);
  auto nkeys = keys2edges.size();
  auto new_mesh = Mesh();
  new_mesh.set_comm(comm);
  new_mesh.set_dim(mesh.dim());
  new_mesh.set_partition(mesh.partition());
  auto keys2midverts = LOs();
  auto same_verts2old_verts = LOs();
  auto same_verts2new_verts = LOs();
  auto old_verts2new_verts = LOs();
  modify_ents(mesh, new_mesh, VERT, EDGE, keys2edges, LOs(nkeys, 0, 1),
      LOs(), LOs(), keys2midverts,
      same_verts2old_verts, same_verts2new_verts,
      old_verts2new_verts);
  return true;
}
