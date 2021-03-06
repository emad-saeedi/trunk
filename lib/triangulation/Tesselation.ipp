/*************************************************************************
*  Copyright (C) 2006 by Bruno Chareyre                                *
*  bruno.chareyre@hmg.inpg.fr                                            *
*                                                                        *
*  This program is free software; it is licensed under the terms of the  *
*  GNU General Public License v2 or later. See file LICENSE for details. *
*************************************************************************/

//FIXME: handle that a better way
#define MAX_ID 200000

namespace CGT {

using std::cerr;
using std::cout;
using std::endl;
using std::max;
using std::vector;
using std::ifstream;

template<class TT>
_Tesselation<TT>::_Tesselation ( void )
{
	Tri = new RTriangulation;
	Tes = Tri;
	computed=false;
	maxId = -1;
	TotalFiniteVoronoiVolume=0;
	area=0;
	TotalInternalVoronoiPorosity=0;
	TotalInternalVoronoiVolume=0;
	redirected = false;
	//FIXME : find a better way to avoid segfault when insert() is used before resizing this vector
	vertexHandles.resize(MAX_ID+1,NULL);
}
template<class TT>
_Tesselation<TT>::_Tesselation ( RTriangulation &T ) : Tri ( &T ), Tes ( &T ), computed ( false )
{
	std::cout << "Tesselation(RTriangulation &T)" << std::endl;
	compute();
}

template<class TT>
_Tesselation<TT>::~_Tesselation ( void )
{
	if ( Tri ) Tri->~RTriangulation();
}
template<class TT>
void _Tesselation<TT>::Clear ( void )
{
	Tri->clear();
	redirected = false;
	vertexHandles.clear();
	vertexHandles.resize(MAX_ID+1,NULL);
	maxId=0;
}
template<class TT>
typename _Tesselation<TT>::VertexHandle _Tesselation<TT>::insert ( Real x, Real y, Real z, Real rad, unsigned int id, bool isFictious )
{
	VertexHandle Vh;
	Vh = Tri->insert(Sphere(Point(x,y,z),pow(rad,2)));
	if ( Vh!=NULL )
	{
		Vh->info().setId(id);
		Vh->info().isFictious = isFictious;
		assert (vertexHandles.size()>id);
		vertexHandles[id] = Vh;
		/*if ( !isFictious ) */maxId = std::max ( maxId, (int) id );
	}
	else cout << id <<  " : Vh==NULL!!"<< " id=" << id << " Point=" << Point ( x,y,z ) << " rad=" << rad << endl;
	return Vh;
}
template<class TT>
typename _Tesselation<TT>::VertexHandle _Tesselation<TT>::move ( Real x, Real y, Real z, Real rad, unsigned int id )
{
	bool fictious = vertexHandles[id]->info().isFictious;
	VertexHandle Vh;
	Vh = Tri->move_point ( vertexHandles[id], Sphere ( Point ( x,y,z ),pow ( rad,2 ) ) );
	if ( Vh!=NULL )
	{
		vertexHandles[id] = Vh;
		Vh->info().setId(id);
		Vh->info().isFictious = fictious;
	}
	else cerr << "Vh==NULL" << " id=" << id << " Point=" << Point ( x,y,z ) << " rad=" << rad << endl;
	return Vh;
}


template<class TT>
bool _Tesselation<TT>::redirect ( void )
{
	if ( !redirected )
	{
		//Set size of the redirection vector
		if ( (unsigned int)maxId+1 != vertexHandles.size() ) vertexHandles.resize ( maxId+1,NULL );
		maxId = 0;
		FiniteVerticesIterator verticesEnd = Tri->finite_vertices_end ();
		for ( FiniteVerticesIterator vIt = Tri->finite_vertices_begin (); vIt !=  verticesEnd; vIt++ )
		{
			vertexHandles[vIt->info().id()]= vIt;
			maxId = max(maxId, (int) vIt->info().id());
		}
		if ( (unsigned int)maxId+1 != vertexHandles.size() ) vertexHandles.resize ( maxId+1 );
		redirected = true;
	} else return false;
	return true;
}

template<class TT>
bool _Tesselation<TT>::remove ( unsigned int id )
{
	redirect();
	Tri->remove ( vertexHandles[id] );
	return true;
}

template<class TT>
void _Tesselation<TT>::voisins ( VertexHandle v, VectorVertex& Output_vector )
{ Tri->incident_vertices ( v, back_inserter ( Output_vector ) ); }

template<class TT>
typename _Tesselation<TT>::RTriangulation & _Tesselation<TT>::Triangulation ( void )
{ return *Tri; }

template<class TT>
Real _Tesselation<TT>::Volume ( FiniteCellsIterator cell )
{
	return ( Tetrahedron ( cell->vertex ( 0 )->point(), cell->vertex ( 1 )->point(),
						 cell->vertex ( 2 )->point(), cell->vertex ( 3 )->point() ) ).volume();
}

template<class TT>
Plane _Tesselation<TT>::Dual ( VertexHandle S1, VertexHandle S2 )
{
	Segment seg ( S1->point(), S2->point() );
	Real r = 0.5* ( 1.0 + ( ( S1->point() ).weight() * ( S1->point() ).weight() - ( S2->point() ).weight() * ( S2->point() ).weight() ) /seg.squared_length() );
	return Plan ( S1->point() + seg.to_vector() *r, seg.to_vector() );
}

template<class TT>
Point _Tesselation<TT>::Dual ( const CellHandle &cell )
{
	return cell->info();
}

template<class TT>
void _Tesselation<TT>::compute ()
{
	if (!redirected) redirect();
	FiniteCellsIterator cellEnd = Tri->finite_cells_end();
	for ( FiniteCellsIterator cell = Tri->finite_cells_begin(); cell != cellEnd; cell++ ) cell->info().setPoint(circumCenter(cell));
// 	{
// 
// 		const Sphere& S0 = cell->vertex ( 0 )->point();
// 		const Sphere& S1 = cell->vertex ( 1 )->point();
// 		const Sphere& S2 = cell->vertex ( 2 )->point();
// 		const Sphere& S3 = cell->vertex ( 3 )->point();
// 		Real x,y,z;
// 		CGAL::weighted_circumcenterC3 (
// 			S0.point().x(), S0.point().y(), S0.point().z(), S0.weight(),
// 			S1.point().x(), S1.point().y(), S1.point().z(), S1.weight(),
// 			S2.point().x(), S2.point().y(), S2.point().z(), S2.weight(),
// 			S3.point().x(), S3.point().y(), S3.point().z(), S3.weight(),
// 			x, y, z );
// 		cell->info().setPoint(Point(x,y,z));
// 	}
	computed = true;
}

template<class TT>
Point _Tesselation<TT>::circumCenter (const CellHandle& cell)
{
	const Sphere& S0 = cell->vertex ( 0 )->point();
	const Sphere& S1 = cell->vertex ( 1 )->point();
	const Sphere& S2 = cell->vertex ( 2 )->point();
	const Sphere& S3 = cell->vertex ( 3 )->point();
	Real x,y,z;
	CGAL::weighted_circumcenterC3 (
		S0.point().x(), S0.point().y(), S0.point().z(), S0.weight(),
		S1.point().x(), S1.point().y(), S1.point().z(), S1.weight(),
		S2.point().x(), S2.point().y(), S2.point().z(), S2.weight(),
		S3.point().x(), S3.point().y(), S3.point().z(), S3.weight(),
		x, y, z );
	return Point(x,y,z);
}
template<class TT>
Point _Tesselation<TT>::setCircumCenter (const CellHandle& cell,bool force) {
	if (force or cell->info()==CGAL::ORIGIN) cell->info().setPoint(circumCenter(cell));
	return (Point) cell->info();
}

template<class TT>
std::vector<int> _Tesselation<TT>::getAlphaVertices(double alpha)
{
	cerr<<"Warning: this is extremely slow - only for experiments"<<endl;
	RTriangulation temp(*Tri);
	AlphaShape as (temp);
	if (!alpha) alpha=as.find_alpha_solid();
	as.set_alpha(alpha);
	std::list<VertexHandle> alphaVertices;
	as.get_alpha_shape_vertices(std::back_inserter(alphaVertices), AlphaShape::REGULAR);
	std::vector<int> res;
	for (auto v=alphaVertices.begin(); v!=alphaVertices.end(); v++) res.push_back((*v)->info().id());
	return res;
}	
	
template<class TT>
void _Tesselation<TT>::testAlphaShape(double alpha)
{
// 	if (not computed) compute();
	
	RTriangulation temp(*Tri);
	AlphaShape as (temp);
	if (!alpha) alpha=as.find_alpha_solid();
	as.set_alpha(alpha);	
	cerr << "Alpha shape computed. alpha_solid=" <<alpha <<endl;
	
	std::list<CellHandle> cells,cells2,cells3,cells4;
	std::list<Facet> facets,facets2,facets3;
	std::list<VertexHandle> alphaVertices;
	std::list<CVector> normals;
	std::list<CVector> normals2;
	std::list<Edge> edges0,edges1,edges2,edges3;
	as.get_alpha_shape_cells(std::back_inserter(cells),AlphaShape::REGULAR);
	as.get_alpha_shape_cells(std::back_inserter(cells2), AlphaShape::EXTERIOR);
	as.get_alpha_shape_cells(std::back_inserter(cells3),AlphaShape::INTERIOR);
	as.get_alpha_shape_cells(std::back_inserter(cells4),AlphaShape::SINGULAR);
	as.get_alpha_shape_facets(std::back_inserter(facets), AlphaShape::REGULAR);
	as.get_alpha_shape_vertices(std::back_inserter(alphaVertices), AlphaShape::REGULAR);
	as.get_alpha_shape_edges(std::back_inserter(edges0), AlphaShape::INTERIOR);
	as.get_alpha_shape_edges(std::back_inserter(edges1), AlphaShape::REGULAR);
	as.get_alpha_shape_edges(std::back_inserter(edges2), AlphaShape::SINGULAR);
	as.get_alpha_shape_edges(std::back_inserter(edges3), AlphaShape::EXTERIOR);
	
	int finitEdges=0;
	for ( FiniteEdgesIterator ed_it=Tri->finite_edges_begin(); ed_it!=Tri->finite_edges_end();ed_it++ ) ++finitEdges;
	
	std::cerr<< "num regular cells "<< cells.size() <<" vs. "<<cells2.size() <<" vs. "<<cells3.size()<<" vs. "<<cells4.size()<<std::endl;
	std::cerr<< "num regular facets "<< facets.size() << std::endl;
	std::cerr<< "num edges "<< edges0.size() <<" "<< edges1.size() <<" "<< edges2.size() <<" "<< edges3.size() <<"(finite ones:"<<finitEdges<<")" << std::endl;
// 	for (auto v=alphaVertices.begin(); v!=alphaVertices.end();v++){
// 		std::cerr<< "alpha vertex:"<<(*v)->info().id()<<std::endl;
// 	}
	for (auto e=edges1.begin(); e!=edges1.end();e++){
		std::cerr<< "alpha edge:"<<e->first->vertex(e->second)->info().id()<<" "<<e->first->vertex(e->third)->info().id() <<" "<< alphaVoronoiFaceArea(*e,as,*Tri) <<std::endl;
	}
// 	for (auto c=cells.begin(); c!=cells.end();c++){
// 		(*c)->info().setPoint(circumCenter(*c));
// 		std::cerr<< "alpha cell:"<<(Point) (*c)->info()<<std::endl;
// 	}
	
	for (auto f=facets.begin(); f!=facets.end();f++){
		const int& idx = f->second;//index of the facet within cell defined by f->first
// 		std::cerr << f->first->vertex(facetVertices[idx][0])->info().id()
// 			<<" "<< f->first->vertex(facetVertices[idx][1])->info().id()
// 			<<" "<< f->first->vertex(facetVertices[idx][2])->info().id()  << std::endl;
		CVector surface = 0.5*cross_product(f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][1])->point().point(),
			f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][2])->point().point());
		//largest sphere
		double maxWeight = std::max(f->first->vertex(facetVertices[idx][0])->point().weight(),max(f->first->vertex(facetVertices[idx][1])->point().weight(), f->first->vertex(facetVertices[idx][2])->point().weight()));
		Point pp;
		Point vv;
 		if (as.classify(f->first)==AlphaShape::INTERIOR) {
			pp= f->first->vertex(f->second)->point();
			if (not computed) f->first->info().setPoint(circumCenter(f->first));
// 			std::cerr<< "alpha cell:"<<(Point) f->first->info()<<std::endl;
			vv = f->first->info();
// 			std::cerr << "vv="<<vv<<std::endl;
// 			if (not computed) f->first->info().setPoint(Tri->dual(f->first));
// 			std::cerr << "found as.classify(f->first)==Alpha_shape_3::INTERIOR"<<std::endl;
		}
		else {
			if (as.classify(f->first->neighbor(f->second))!=AlphaShape::INTERIOR) std::cerr<<"_____________BIG PROB. HERE ___________"<<std::endl;

			pp= f->first->neighbor(f->second)->vertex(Tri->mirror_index(f->first,f->second))->point();
			if (not computed) f->first->neighbor(f->second)->info().setPoint(circumCenter(f->first->neighbor(f->second)));
// 			std::cerr<< "alpha cell:"<<(Point) f->first->neighbor(f->second)->info()<<std::endl;
			vv = f->first->neighbor(f->second)->info();
// 			std::cerr << "vv="<<vv<<std::endl;
// 			if (not computed) f->first->neighbor(f->second)->info().setPoint(Tri->dual(f->first->neighbor(f->second)));
// 			std::cerr << "not an Alpha_shape_3::INTERIOR"<<std::endl;
		}
		//check if the surface vector is inward or outward
		double dotP = surface*(f->first->vertex(facetVertices[f->second][0])->point()-pp);
		if (dotP<0) surface=-surface;
		double area = sqrt(surface.squared_length());
		CVector normal = surface/area; //unit normal
// 		std::cerr <<"dotP="<<dotP<<std::endl<<"surface: "<<surface<<std::endl;
		
		double h1 = (f->first->vertex(facetVertices[idx][0])->point().point()-vv)*surface/area; //orthogonal distance from Voronoi vertex to the plane in which the spheres lie, call the intersection V
		Point V = vv + h1*normal;
		double distLiu = sqrt((V-Point(0,0,0)).squared_length());
		double sqR = (V-f->first->vertex(facetVertices[idx][0])->point().point()).squared_length(); //squared distance between V and the center of sphere 0 
		double temp = alpha + f->first->vertex(facetVertices[idx][0])->point().weight() -sqR;
		if (temp<0) {temp=0; std::cerr<<"NEGATIVE TEMP!"<<std::endl;}
		if (temp>maxWeight) temp=maxWeight; //if alpha vertex is too far, crop
		double h2 = sqrt(temp);// this is now the distance from Voronoi vertex to "alpha" vertex (after cropping if needed)
		V = V+h2*normal;
		std::cerr <<"dist alpha center:"<<sqrt((V-Point(0,0,0)).squared_length())<<"(vs. Liu:"<< distLiu << ")"<<std::endl;
	}
}

// template<class TT>
// void _Tesselation<TT>::setAlphaBoundaryPoints(double alpha)
// {
// 	AlphaShape as (*Tri);
// 	if (!alpha) as.set_alpha(as.find_alpha_solid());
// 	else as.set_alpha(alpha);
// 	cerr << "Alpha shape computed. alpha_solid=" <<as.find_alpha_solid() <<endl;
// 	std::list<CellHandle> cells,cells2,cells3;
// 	std::list<Facet> facets,facets2,facets3;
// 	std::list<CVector> normals;
// 	as.get_alpha_shape_cells(std::back_inserter(cells),AlphaShape::EXTERIOR);
// 	as.get_alpha_shape_cells(std::back_inserter(cells2), AlphaShape::INTERIOR);
// 	as.get_alpha_shape_facets(std::back_inserter(facets), AlphaShape::REGULAR);
// 	
// 	faces.resize(facets.size()); int k=0;
// 
// 	std::cerr<< "num exterior cells "<< cells.size() <<" vs. "<<cells2.size() <<std::endl;
// 	std::cerr<< "num regular facets "<< facets.size() << std::endl;
// 	for (auto f=facets.begin(); f!=facets.end();f++){
// 		int idx = f->second;//index of the facet within cell defined by f->first
// 		std::cerr << f->first->vertex(facetVertices[idx][0])->info().id()
// 			<<" "<< f->first->vertex(facetVertices[idx][1])->info().id()
// 			<<" "<< f->first->vertex(facetVertices[idx][2])->info().id()  << std::endl;
// 		CVector normal = 0.5*cross_product(f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][1])->point().point(),
// 			f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][2])->point().point());
// 		Point pp;
//  		if (as.classify(f->first)==AlphaShape::INTERIOR) {
// 			pp= f->first->vertex(f->second)->point(); std::cerr << "found as.classify(f->first)==Alpha_shape_3::INTERIOR"<<std::endl;}
// 		else {
// 			pp= f->first->neighbor(f->second)->vertex(Tri->mirror_index(f->first,f->second))->point(); std::cerr << "not an Alpha_shape_3::INTERIOR"<<std::endl;}
// 		//check if the normal vector is inward or outward
// 		double dotP = normal*(f->first->vertex(facetVertices[f->second][0])->point()-pp);
// 		if (dotP<0) normal=-normal;
// 		// set the face in the global list 
// 		for (int ii=0; ii<3;ii++) faces[k].ids[ii]= f->first->vertex(facetVertices[idx][ii])->info().id();
// 		faces[k++].normal = normal;
// 		std::cerr <<"dotP="<<dotP<<std::endl<<"normal: "<<normal<<std::endl;
// 	}
// }

template<class TT>
void _Tesselation<TT>::setAlphaFaces(std::vector<AlphaFace>& faces, double alpha)
{
	RTriangulation temp(*Tri);
	AlphaShape as (temp);
	if (!alpha) {
		as.set_alpha(as.find_alpha_solid());
		/*cerr << "Alpha shape computed. alpha_solid=" <<as.find_alpha_solid() <<endl;*/}
	else as.set_alpha(alpha);
	
	std::list<Facet> facets;
	std::list<CVector> normals;
	as.get_alpha_shape_facets(std::back_inserter(facets), AlphaShape::REGULAR);// get the list of "contour" facets
	faces.resize(facets.size()); int k=0;

	for (auto f=facets.begin(); f!=facets.end();f++){
		int idx = f->second;//index of the facet within cell defined by f->first
		CVector normal = 0.5*cross_product(f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][1])->point().point(),
			f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][2])->point().point());
		Point pp;
 		if (as.classify(f->first)==AlphaShape::INTERIOR) pp= f->first->vertex(f->second)->point();
		else pp= f->first->neighbor(f->second)->vertex(Tri->mirror_index(f->first,f->second))->point();
		//check if the normal vector is inward or outward
		double dotP = normal*(f->first->vertex(facetVertices[f->second][0])->point()-pp);
		if (dotP<0) normal=-normal;
		// set the face in the global list 
		for (int ii=0; ii<3;ii++) faces[k].ids[ii]= f->first->vertex(facetVertices[idx][ii])->info().id();
		faces[k++].normal = normal;
	}
}

template<class TT>
void _Tesselation<TT>::setExtendedAlphaCaps ( std::vector<AlphaCap>& faces, double alpha )
{
  RTriangulation temp ( *Tri );
  AlphaShape as ( temp );
  if ( !alpha ) as.set_alpha ( as.find_alpha_solid() );
  else as.set_alpha ( alpha );


  std::list<VertexHandle> alphaVertices;
  std::list<Edge> edges;
  std::vector<CVector> areas;
  as.get_alpha_shape_vertices ( std::back_inserter ( alphaVertices ), AlphaShape::REGULAR );
  as.get_alpha_shape_edges ( std::back_inserter ( edges ), AlphaShape::REGULAR ); // get the list of "contour" facets
  //initialize area vectors in a list where spheres defined by their ids (hence the size)
  areas.resize ( as.number_of_vertices(),CVector ( 0,0,0 ) );// from 0 to maxId
  faces.resize ( alphaVertices.size() ); // this one will keep only the boundary particles, listed by {id,area}

  for ( auto e=edges.begin(); e!=edges.end(); e++ )
    {
      
      CVector area = alphaVoronoiPartialCapArea ( *e,as );
      cerr <<"Edge:"<<e->first->vertex ( e->second )->info().id()<<" to "<<e->first->vertex ( e->third )->info().id()<<" = "<<area <<endl;
      areas[e->first->vertex ( e->second )->info().id()]= areas[e->first->vertex ( e->second )->info().id()] + area;
      areas[e->first->vertex ( e->third )->info().id()]= areas[e->first->vertex ( e->third )->info().id()] - area;
    }
  int k=0;
  for ( auto body=alphaVertices.begin(); body!=alphaVertices.end(); body++ )
    {
      const unsigned int& id = ( *body )->info().id();
      faces[k].id=id;
      faces[k].normal=areas[id];
      ++k;
    }
}

// template<class TT>
// void _Tesselation<TT>::setAlphaFacesExtended(std::vector<AlphaFace>& faces, double alpha)
// {
// 	RTriangulation temp(*Tri);
// 	AlphaShape as (temp);
// 	if (!alpha) {
// 		as.set_alpha(as.find_alpha_solid());
// 		/*cerr << "Alpha shape computed. alpha_solid=" <<as.find_alpha_solid() <<endl;*/}
// 	else as.set_alpha(alpha);
// 	
// 	std::list<Facet> facets;
// 	std::list<CVector> normals;
// 	as.get_alpha_shape_facets(std::back_inserter(facets), AlphaShape::REGULAR);// get the list of "contour" facets
// 	faces.resize(facets.size()); int k=0;
// 
// 	for (auto f=facets.begin(); f!=facets.end();f++){
// 		int idx = f->second;//index of the facet within cell defined by f->first
// 		CVector normal = 0.5*cross_product(f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][1])->point().point(),
// 			f->first->vertex(facetVertices[idx][0])->point().point()-f->first->vertex(facetVertices[idx][2])->point().point());
// 		Point pp;
//  		if (as.classify(f->first)==AlphaShape::INTERIOR) pp= f->first->vertex(f->second)->point();
// 		else pp= f->first->neighbor(f->second)->vertex(Tri->mirror_index(f->first,f->second))->point();
// 		//check if the normal vector is inward or outward
// 		double dotP = normal*(f->first->vertex(facetVertices[f->second][0])->point()-pp);
// 		if (dotP<0) normal=-normal;
// 		// set the face in the global list 
// 		for (int ii=0; ii<3;ii++) faces[k].ids[ii]= f->first->vertex(facetVertices[idx][ii])->info().id();
// 		faces[k++].normal = normal;
// 	}
// }

template<class TT>
Segment _Tesselation<TT>::Dual ( FiniteFacetsIterator &f_it )
{
	return Segment ( f_it->first->info(), ( f_it->first->neighbor ( f_it->second ) )->info() );
}

template<class TT>
double _Tesselation<TT>::computeVFacetArea ( FiniteEdgesIterator ed_it )
{
	CellCirculator cell0 = Tri->incident_cells ( *ed_it );
	CellCirculator cell2 = cell0;

	if ( Tri->is_infinite ( cell2 ) ){
		++cell2;
		while ( Tri->is_infinite ( cell2 ) && cell2!=cell0 ) ++cell2;
		if ( cell2==cell0 ) return 0;
	}
	cell0=cell2++;
	CellCirculator cell1=cell2++;
	Real area = 0;

	while ( cell2!=cell0 ){
	  	area+= sqrt(std::abs (( Triangle ( cell0->info(), cell1->info(), cell2->info() ) ).squared_area())) ;
		++cell1;
		++cell2;
	}
	return area;
}

template<class TT>
CVector _Tesselation<TT>::alphaVoronoiFaceArea (const Edge& ed_it, const AlphaShape& as, const RTriangulation& Tro)
{
	//Overall, we calculate the area vector of the polygonal Voronoi face between two spheres, this is done by integrating x×dx
 
        double alpha = as.get_alpha();
	CellCirculator cell0,cell1,cell2; 
	cell0 = as.incident_cells ( ed_it );
	cell2 = cell0;
	while ( as.classify(cell2)!=AlphaShape::INTERIOR ) {++cell2; if (cell2==cell0) cerr<<"infinite loop on an edge, probably singular"<<endl;}
	cell1=cell2;

	//pA,pB are the spheres of the edge, (p1,p2) are iterating over the vertices of the vornonoi face, p12 can be an intermediate point for EXTERIOR-EXTERIOR parts of the contour 
	Point pA,pB,p1,p2,p12,vv0,vv;
        p1 = setCircumCenter(cell1);//starting point of the polygon
	CVector branch, normal;
	CVector branchArea(0,0,0);
	pA = ( ed_it.first )->vertex ( ed_it.second )->point().point();//one sphere
	pB = ( ed_it.first )->vertex ( ed_it.third )->point().point();//another sphere
	CVector AB = pB-pA;
	bool interior1 = true;//keep track of last cell's status
	bool interior2; 
	do {
		++cell2;
		interior2 = (as.classify(cell2)==AlphaShape::INTERIOR);
		if (interior2) {//easy
			setCircumCenter(cell2);
			p2 = cell2->info();
			branch=p2-p1;
			branchArea = branchArea + cross_product(branch, p1-CGAL::ORIGIN);
			cerr<<"branchArea(1) "<<branch<<" cross "<<p1<<endl;
		} else {//tricky, we have to construct the face between INTERIOR-EXTERIOR, or even EXTERIOR-EXTERIOR
			CellCirculator baseCell=cell1;
			//handle EXTERIOR-EXTERIOR by checking the n+1 cell
			if (!interior1) {baseCell=cell2; baseCell++; vv0=vv;if(as.classify(baseCell)!=AlphaShape::INTERIOR) cerr<<"3 consecutive EXTERIOR cells in a loop";}
			vv = setCircumCenter(baseCell);
			// finding the facet from baseCell to cell2 ...
			int idx=0; while (baseCell->neighbor(idx)!=cell2) {idx++; if(idx>3) cerr<<"HUUUUUUUH";}
			// ... then its surface vector
			CVector surface = 0.5*cross_product(baseCell->vertex(facetVertices[idx][0])->point().point()-baseCell->vertex(facetVertices[idx][1])->point().point(),
			baseCell->vertex(facetVertices[idx][0])->point().point()-baseCell->vertex(facetVertices[idx][2])->point().point());
			//largest sphere
			double maxWeight = std::max(baseCell->vertex(facetVertices[idx][0])->point().weight(),max(baseCell->vertex(facetVertices[idx][1])->point().weight(), baseCell->vertex(facetVertices[idx][2])->point().weight()));
			//check if the surface vector is inward or outward
			double dotP = surface*(baseCell->vertex(facetVertices[idx][0])->point()-baseCell->vertex(idx)->point());
			if (dotP<0) surface=-surface;
			double area = sqrt(surface.squared_length());
			normal = surface/area; //unit normal
			double h1 = (baseCell->vertex(facetVertices[idx][0])->point().point()-vv)*normal; //orthogonal distance from Voronoi vertex to the plane in which the spheres lie, call the intersection V
			p2 = vv + h1*normal;
			double sqR = (p2-baseCell->vertex(facetVertices[idx][0])->point().point()).squared_length(); //squared distance between V and the center of sphere 0
                        double temp = alpha + baseCell->vertex(facetVertices[idx][0])->point().weight() -sqR;
                        if (temp<0) {temp=0; std::cerr<<"NEGATIVE TEMP!"<<std::endl;}
                        if (temp>maxWeight) temp=maxWeight; //if alpha vertex is too far, crop
                        double h2 = sqrt(temp);// this is now the distance from Voronoi vertex to "alpha" vertex (after cropping if needed)
		        p2 = p2+h2*normal;
		
		        bool coplanar=false;
		
		        if (!(interior1 or interior2))  {
			        //VERSION 1,intersection of orthogonal planes from two branches
			        CVector tangent = cross_product(AB,p1-vv0);
			        tangent = tangent/sqrt(tangent.squared_length());//this is orthogonal to the _previous_ branch segment of the polygonal contour
			        double dotP = tangent*normal;
			        coplanar=(abs(dotP)<1e-2);
				CVector p1mp2=p1-p2;
			        if (!coplanar) {
					//make sure the construction is not singular (no intermediate vertex)
					if ((p1mp2*(p1-vv0)>0) and (p1mp2*normal<0)) {
					p12=p1-(p1mp2)*normal/dotP*tangent;
					
// 					if (((p12-p1)*p1mp2)*(p12-p2)*p1mp2)<0) {//make sure the construction is not singular (no intermediate vertex)
			        cerr<<"p12="<<p12<<" with p1="<< p1<<", p2="<<p2 <<" tangent="<<tangent<<" "<<AB <<" "<<p1-pA <<endl;
			        //VERSION 2... a different p12 (possibly parallelogram?)
			
			        //Whatever the method:
			        branchArea = branchArea+cross_product(p12-p1, p1-CGAL::ORIGIN);
				cerr<<"branchArea(2) "<<p12-p1<<" cross "<<p1<<endl;
			        p1 = p12;}
				}
                        }
                        cerr<<"branchArea(3) "<<p2-p1<<" cross "<<p1<<endl;
		        branchArea = branchArea+cross_product(p2-p1, p1-CGAL::ORIGIN);
			
		}
		cell1=cell2; p1=p2; interior1=interior2;
		
	} while (cell2!=cell0);
	return 0.5*branchArea;//0.5 because the above integral is twice the area
}

template<class TT>
CVector _Tesselation<TT>::alphaVoronoiPartialCapArea ( const Edge& ed_it, const AlphaShape& as )
{
	//Overall, a partial area vector based on only the outer part of a polygonal Voronoi face (see alphaVoronoiFaceArea), looping on REGULAR edges incident to a boundary sphere and using this function for each of them should give the contour integral of x×dx for the polygonal cap of the sphere.
	double alpha = as.get_alpha();
	CellCirculator cell0,cell1,cell2,cell3;
	cell0 = as.incident_cells ( ed_it );
	cell2 = cell0;

	////TEMP
	++cell2;
	while ( cell2 !=cell0 ) {
		cerr <<"cell cirulator:"<<(as.classify ( cell2 ) ==AlphaShape::INTERIOR)<<" "<<as.is_infinite(cell2)<<endl;
		++cell2;
	//     if ( cell2==cell0 ) cerr<<"infinite loop on an edge, probably singular"<<endl;
	}
	cerr <<"cell cirulator:"<<(as.classify ( cell0 ) ==AlphaShape::INTERIOR)<<" "<<as.is_infinite(cell0)<<endl;
//// END TEMP
	while ( as.classify ( cell2 ) !=AlphaShape::INTERIOR ){
		++cell2;
		if ( cell2==cell0 ) cerr<<"infinite loop on an edge, probably singular"<<endl;
	}
	cell0=cell1=cell3=cell2;
	cell3++;

	//pA,pB are the spheres of the edge, (p1,p2) are iterating over the vertices of the vornonoi face, p12 can be an intermediate point for EXTERIOR-EXTERIOR parts of the contour
	Point pA,pB,p1,p2,p12,vv0,vv;
	//   p1 = setCircumCenter ( cell1 ); //starting point of the polygon
	CVector branch, normal;
	CVector branchArea ( 0,0,0 );
	pA = ( ed_it.first )->vertex ( ed_it.second )->point().point();//one sphere
	pB = ( ed_it.first )->vertex ( ed_it.third )->point().point();//another sphere
	CVector AB = pB-pA;
	bool interior1 = true;//keep track of last cell's status
	bool interior2,interior3,isFiniteAlphaCell;
	interior2=true;
	do {
		if (interior2 or as.is_infinite(cell2)) {//if on a finite alpha cell we have multiple voronoi vertices in one single cell, hence no stepping in that case
			++cell2; ++cell3;
			interior2 = ( as.classify ( cell2 ) ==AlphaShape::INTERIOR );
			interior3 = ( as.classify ( cell3 ) ==AlphaShape::INTERIOR );
			isFiniteAlphaCell=false;
			} else isFiniteAlphaCell=true;
		if ( !interior2 ) {
//                         if (not interior3) p2=setCircumCenter(cell2);//prepare the starting point in the last INTERIOR cell
// //                         p2 = cell2->info();
// //                         branch=p2-p1;
// //                         branchArea = branchArea + cross_product(branch, p1-CGAL::ORIGIN);
// //                         cerr<<"branchArea(1) "<<branch<<" cross "<<p1<<endl;
//                 } else {//we have to construct the external line or poly-line between two EXTERIOR cells
			CellCirculator baseCell=cell1;
			//handle EXTERIOR-EXTERIOR by checking the n+1 cell
			if ( !interior1 ) {
				baseCell=cell2;
				baseCell++;
				vv0=vv;
				cerr<<"archiving vv0 "<<vv0<<endl;
				if ( as.classify ( baseCell ) !=AlphaShape::INTERIOR ) cerr<<"3 consecutive EXTERIOR cells in a loop";
			}
			vv = setCircumCenter ( baseCell );
			cerr<<"circumCenter"<<vv<<endl;
			// finding the facet from baseCell to cell2 ...
			int idx=0;
			while ( baseCell->neighbor ( idx ) !=cell2 ) {
				idx++;
				if ( idx>3 ) cerr<<"HUUUUUUUH";}
			// ... then its surface vector
			CVector surface = 0.5*cross_product ( baseCell->vertex ( facetVertices[idx][0] )->point().point()-baseCell->vertex ( facetVertices[idx][1] )->point().point(),
			baseCell->vertex ( facetVertices[idx][0] )->point().point()-baseCell->vertex ( facetVertices[idx][2] )->point().point() );
			//largest sphere
			double maxWeight = std::max ( baseCell->vertex ( facetVertices[idx][0] )->point().weight(),max ( baseCell->vertex ( facetVertices[idx][1] )->point().weight(), baseCell->vertex ( facetVertices[idx][2] )->point().weight() ) );
			//check if the surface vector is inward or outward
			double dotP = surface* ( baseCell->vertex ( facetVertices[idx][0] )->point()-baseCell->vertex ( idx )->point() );
			if ( dotP<0 ) surface=-surface;
			double area = sqrt ( surface.squared_length() );
			normal = surface/area; //unit normal
			double h1 = ( baseCell->vertex ( facetVertices[idx][0] )->point().point()-vv ) *normal; //orthogonal distance from Voronoi vertex to the plane in which the spheres lie, call the intersection V
			p2 = vv + h1*normal;
			double sqR = ( p2-baseCell->vertex ( facetVertices[idx][0] )->point().point() ).squared_length(); //squared distance between V and the center of sphere 0
			double temp = alpha + baseCell->vertex ( facetVertices[idx][0] )->point().weight() -sqR;
			if ( temp<0 ){
				temp=0;
				std::cerr<<"NEGATIVE TEMP!"<<std::endl;}
			if ( temp>maxWeight ) temp=maxWeight; //if alpha vertex is too far, crop
			double h2 = sqrt ( temp ); // this is now the distance from Voronoi vertex to "alpha" vertex (after cropping if needed)
			p2 = p2+h2*normal;

			if ( ! ( interior1 or interior2 ) ){
				//VERSION 1,intersection of orthogonal planes from two branches
				CVector tangent = cross_product ( AB,p1-vv0 );
				CVector p1mp2=p1-p2;
				bool clockWise = ( tangent*p1mp2>0 );
				tangent = tangent/sqrt ( tangent.squared_length() ); //this is orthogonal to the _previous_ branch segment of the polygonal contour
				double dotP = tangent*normal;

				if ( (!isFiniteAlphaCell) and abs(dotP)>1e-2 /*not coplanar*/){
					//make sure the construction is not singular (no intermediate vertex)
					if ( ( p1mp2* ( p1-vv0 ) >0 ) and ( p1mp2*normal<0 ) ){
						p12=p1- ( p1mp2 ) *normal/dotP*tangent;
//						if (((p12-p1)*p1mp2)*(p12-p2)*p1mp2)<0) {//make sure the construction is not singular (no intermediate vertex)
						cerr<<"p12="<<p12<<" with p1="<< p1<<", p2="<<p2 <<" tangent="<<tangent<<" "<<AB <<" "<<p1-pA <<endl;
						//VERSION 2... a different p12 (possibly parallelogram?)

						//Whatever the method:
						branchArea = cross_product ( p12-p1, p1-CGAL::ORIGIN );
						cerr<<"branchArea(2) "<<p12-p1<<" cross "<<p1<<endl;
						p1 = p12;
					}
				}

				cerr<<"branchArea(3) "<<p2-p1<<" cross "<<p1<<endl;
				branchArea = branchArea+cross_product ( p2-p1, p1-CGAL::ORIGIN );

				//check the orientation (we need to accumulate along the polyline with a given direction)
				if ( clockWise ) return 0.5*branchArea;
				else return -0.5*branchArea;
			}
		}
		cell1=cell2;
		p1=p2;
		interior1=interior2;
	}
	while ( cell2!=cell0 );
	cerr << "WE SHOULD NEVER REACH HERE" <<endl;
	return CVector(0,0,0);
}

template<class TT>
void _Tesselation<TT>::AssignPartialVolume ( FiniteEdgesIterator& ed_it )
{
	//EdgeIterator ed_it
	CellCirculator cell0=Tri->incident_cells ( *ed_it );
	CellCirculator cell2=cell0;
	if ( Tri->is_infinite ( cell2 ) )
	{
		++cell2;
		while ( Tri->is_infinite ( cell2 ) && cell2!=cell0 ) ++cell2;
		if ( cell2==cell0 ) return;
	}
	cell0=cell2++;
	CellCirculator cell1=cell2++;
	bool isFictious1 = ( ed_it->first )->vertex ( ed_it->second )->info().isFictious;
	bool isFictious2 = ( ed_it->first )->vertex ( ed_it->third )->info().isFictious;
	Real r;
	while ( cell2!=cell0 )
	{
		if ( !Tri->is_infinite ( cell1 )  && !Tri->is_infinite ( cell2 ) )
		{
			if ( !isFictious1 )
			{
				r = std::abs ( ( Tetrahedron ( ed_it->first->vertex ( ed_it->second )->point(), cell0->info(), cell1->info(), cell2->info() ) ).volume() );
				( ed_it->first )->vertex ( ed_it->second )->info().v() += r;
				TotalFiniteVoronoiVolume+=r;
			}
			if ( !isFictious2 )
			{
				r = std::abs ( ( Tetrahedron ( ed_it->first->vertex ( ed_it->third )->point(), cell0->info(),  cell1->info(), cell2->info() ) ).volume() );
				ed_it->first->vertex ( ed_it->third )->info().v() +=r;
				TotalFiniteVoronoiVolume+=r;
			}
		}
		++cell1; ++cell2;
	}
}

template<class TT>
void _Tesselation<TT>::ResetVCellVolumes ( void )
{
	for ( VertexIterator  vIt = Tri->vertices_begin (); vIt !=  Tri->vertices_end (); vIt++ ) vIt->info().v() =0;
	TotalFiniteVoronoiVolume=0;
	TotalInternalVoronoiPorosity=0;
}

template<class TT>
void _Tesselation<TT>::computeVolumes ( void )
{
	if ( !computed ) compute();
	ResetVCellVolumes();
	for ( FiniteEdgesIterator ed_it=Tri->finite_edges_begin(); ed_it!=Tri->finite_edges_end();ed_it++ )
	{
		AssignPartialVolume ( ed_it );
	}
	//FIXME: find a way to compute a volume correctly for spheres of the boarders.
}

template<class TT>
void _Tesselation<TT>::computePorosity ( void )  //WARNING : This function will erase real volumes of cells
{
	computeVolumes();
	FiniteVerticesIterator verticesEnd = Tri->finite_vertices_end ();
	for ( FiniteVerticesIterator vIt = Tri->finite_vertices_begin (); vIt !=  verticesEnd; vIt++ )
	{
		if ( vIt->info().v() && !vIt->info().isFictious )
		{
			Real r = 4.188790 * std::pow ( ( vIt->point().weight() ),1.5 );// 4/3*PI*R³ = 4.188...*R³
			TotalInternalVoronoiPorosity+=r;
			TotalInternalVoronoiVolume += vIt->info().v();
			vIt->info().v() =
				( vIt->info().v() - r )
				/ vIt->info().v();
		}
	}
	TotalInternalVoronoiPorosity= ( TotalInternalVoronoiVolume-TotalInternalVoronoiPorosity ) /TotalInternalVoronoiVolume;
}


template<class TT>
bool _Tesselation<TT>::is_internal ( FiniteFacetsIterator &facet )
{
	return ( !Tri->is_infinite ( facet->first ) &&  !Tri->is_infinite ( facet->first->neighbor ( facet->second ) ) );
}
	
	

template<class Tesselation>
typename Tesselation::VertexHandle PeriodicTesselation<Tesselation>::insert(Real x, Real y, Real z, Real rad, unsigned int id, bool isFictious, int duplicateOfId)
{
	VertexHandle Vh;
	if (!Tri) cerr<<"!Tri!"<<endl;
	Vh = Tri->insert(Sphere(Point(x,y,z),pow(rad,2)));
	if ( Vh!=NULL )
	{
		Vh->info() = id;
		Vh->info().isFictious = isFictious;
		if (duplicateOfId<0) {
			assert (vertexHandles.size()>id);
			vertexHandles[id] = Vh;
			maxId = std::max ( maxId, (int) id );
			Vh->info().isGhost=0;
		} else Vh->info().isGhost=1;
	}
	else cerr << " : Vh==NULL!!" << " id=" << id << " Point=" << Point ( x,y,z ) << " rad=" << rad<<" fictious="<<isFictious<<", isGhost="<< bool(duplicateOfId>=0)<<endl;
	return Vh;
}

template<class Tesselation>
bool PeriodicTesselation<Tesselation>::redirect ( void )
{
	if ( !redirected )
	{
		//Set size of the redirection vector
		if ( (unsigned int)maxId+1 != vertexHandles.size() ) vertexHandles.resize ( maxId+1,NULL );
		cout << "!redirected" << endl;
		maxId = 0;
		FiniteVerticesIterator verticesEnd = Tri->finite_vertices_end ();
		for ( FiniteVerticesIterator vIt = Tri->finite_vertices_begin (); vIt !=  verticesEnd; vIt++ )
		{
			if (vIt->info().isGhost) continue;
			vertexHandles[vIt->info().id()]= vIt;
			maxId = max(maxId, (int) vIt->info().id());
		}
		if ( (unsigned int)maxId+1 != vertexHandles.size() ) vertexHandles.resize ( maxId+1 );
		redirected = true;
	} else return false;
	return true;
}

} //namespace CGT
