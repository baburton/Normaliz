/*
 * Normaliz 2.5
 * Copyright (C) 2007-2010  Winfried Bruns, Bogdan Ichim, Christof Soeger
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


//---------------------------------------------------------------------------

#include "integer.h"
#include "vector_operations.h"
#include "matrix.h"
#include "simplex.h"
#include "list_operations.h"

//---------------------------------------------------------------------------

namespace libnormaliz {

//---------------------------------------------------------------------------
//Private
//---------------------------------------------------------------------------
template<typename Integer>
void Simplex<Integer>::reduce_and_insert_interior(const vector< Integer >& new_element){
	//implementing this function as a tree searching may speed up computations ...
	if (new_element[0]==0) {
		return; // new_element=0
	}
	else {
		register int i,c=1,d=dim+1;
		typename list< vector<Integer> >::iterator j;
		for (j =Hilbert_Basis.begin(); j != Hilbert_Basis.end(); j++) {
			if (new_element[0]<2*(*j)[0]) {
				break; //new_element is not reducible;
			}
			else  {
				if ((*j)[c]<=new_element[c]){
					for (i = 1; i < d; i++) {
						if ((*j)[i]>new_element[i]){
							c=i;
							break;
						}
					}
					if (i==d) {
						Hilbert_Basis.push_front(*j);
						Hilbert_Basis.erase(j);
						return;
					}
					//new_element is not in the Hilbert Basis
				}
			}
		}
		Hilbert_Basis.push_back(new_element);
	}
}

//---------------------------------------------------------------------------
//Public
//---------------------------------------------------------------------------

template<typename Integer>
Simplex<Integer>::Simplex(){
	status="non initialized";
}

template<typename Integer>
Simplex<Integer>::Simplex(const vector<int>& k){
	dim=k.size();
	key=k;
	volume=0;
	status="key initialized";
}

template<typename Integer>
Simplex<Integer>::Simplex(const Matrix<Integer>& Map){
	dim=Map.nr_of_columns();
	key=Map.max_rank_submatrix_lex(dim);
	Generators=Map.submatrix(key);
	vector< Integer > help(dim);
	Support_Hyperplanes=Invert(Generators, help, volume); //test for arithmetic
	//overflow performed
	volume=Iabs(volume);
	diagonal=v_abs(help);
	Support_Hyperplanes=Support_Hyperplanes.transpose();
	multiplicators=Support_Hyperplanes.make_prime();
	list< vector<Integer> >  Help;
	Hilbert_Basis=Help;
	status="initialized";
}

//---------------------------------------------------------------------------

template<typename Integer>
Simplex<Integer>::Simplex(const vector<int>& k, const Matrix<Integer>& Map){
	key=k;
	Generators=Map.submatrix(k);
	dim=k.size();
	vector< Integer > help(dim);
	Support_Hyperplanes=Invert(Generators, help, volume);  //test for arithmetic
	//overflow performed
	volume=Iabs(volume);
	diagonal=v_abs(help);
	Support_Hyperplanes=Support_Hyperplanes.transpose();
	multiplicators=Support_Hyperplanes.make_prime();
	list< vector<Integer> >  Help;
	Hilbert_Basis=Help;
	status="initialized";
}

//---------------------------------------------------------------------------

template<typename Integer>
Simplex<Integer>::Simplex(const Simplex<Integer>& S){
	dim=S.dim;
	status=S.status;
	volume=S.volume;
	key=S.key;
	Generators=S.Generators;
	diagonal=S.diagonal;
	multiplicators=S.multiplicators;
	New_Face=S.New_Face;
	Support_Hyperplanes=S.Support_Hyperplanes;
	Hilbert_Basis=S.Hilbert_Basis;
	Homogeneous_Elements=S.Homogeneous_Elements;
	H_Vector=S.H_Vector;
}

//---------------------------------------------------------------------------

template<typename Integer>
Simplex<Integer>::~Simplex(){
	//automatic destructor
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::write_new_face(const vector<int>& Face){
	New_Face=Face;
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::read() const{
	cout<<"\nDimension="<<dim<<"\n";
	cout<<"\nStatus="<<status<<"\n";
	cout<<"\nVolume="<<volume<<"\n";
	cout<<"\nKey is:\n";
	v_read(key);
	cout<<"\nGenerators are:\n";
	Generators.read();
	cout<<"\nDiagonal is:\n";
	v_read(diagonal);
	cout<<"\nMultiplicators are:\n";
	v_read(multiplicators);
	cout<<"\nNew face is:\n";
	v_read(New_Face);
	cout<<"\nSupport Hyperplanes are:\n";
	Support_Hyperplanes.read();
	Matrix<Integer> M=read_hilbert_basis();
	cout<<"\nHilbert Basis is:\n";
	M.read();
	cout<<"\nh-vector is:\n";
	v_read(H_Vector);
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::read_k() const{
	v_read(key);
	v_read(New_Face);
}

//---------------------------------------------------------------------------

template<typename Integer>
int Simplex<Integer>::read_dimension() const{
	return dim;
}

//---------------------------------------------------------------------------

template<typename Integer>
string Simplex<Integer>::read_status() const{
	return status;
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::write_volume(const Integer& vol){
	volume=vol;
}

//---------------------------------------------------------------------------

template<typename Integer>
Integer Simplex<Integer>::read_volume() const{
	return volume;
}

//---------------------------------------------------------------------------

template<typename Integer>
vector<int> Simplex<Integer>::read_key() const{
	return key;
}

//---------------------------------------------------------------------------

template<typename Integer>
Matrix<Integer> Simplex<Integer>::read_generators() const{
	return Generators;
}

//---------------------------------------------------------------------------

template<typename Integer>
vector<Integer> Simplex<Integer>::read_diagonal() const{
	return diagonal;
}

//---------------------------------------------------------------------------

template<typename Integer>
vector<Integer> Simplex<Integer>::read_multiplicators() const{
	return multiplicators;
}

//---------------------------------------------------------------------------

template<typename Integer>
vector<int> Simplex<Integer>::read_new_face() const{
	return New_Face;
}

//---------------------------------------------------------------------------

template<typename Integer>
int Simplex<Integer>::read_new_face_size() const{
	return New_Face.size();
}

//---------------------------------------------------------------------------

template<typename Integer>
Matrix<Integer> Simplex<Integer>::read_support_hyperplanes() const{
	return Support_Hyperplanes;
}

//---------------------------------------------------------------------------

template<typename Integer>
Matrix<Integer> Simplex<Integer>::read_hilbert_basis()const{
	int s= Hilbert_Basis.size();
	Matrix<Integer> M(s,dim);
	int i=1;
	typename list< vector<Integer> >::const_iterator l;
	for (l =Hilbert_Basis.begin(); l != Hilbert_Basis.end(); l++) {
		M.write(i,(*l));
		i++;
	}
	return M;
}

//---------------------------------------------------------------------------

template<typename Integer>
list< vector<Integer> > Simplex<Integer>::read_homogeneous_elements()const{
	list< vector<Integer> > HE=Homogeneous_Elements;
	return HE;
}

//---------------------------------------------------------------------------

template<typename Integer>
const list< vector<Integer> >& Simplex<Integer>::acces_hilbert_basis()const{
	const list< vector<Integer> >& HB=Hilbert_Basis;
	return HB;
}

//---------------------------------------------------------------------------

template<typename Integer>
vector<Integer> Simplex<Integer>::read_h_vector() const{
	return H_Vector;
}

//---------------------------------------------------------------------------

template<typename Integer>
int Simplex<Integer>::read_hilbert_basis_size() const{
	return Hilbert_Basis.size();
}

//---------------------------------------------------------------------------

template<typename Integer>
int Simplex<Integer>::compare(const Simplex<Integer>& S) const{
	return v_difference_ordered_fast(key,S.key);
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::initialize(const Matrix<Integer>& Map){
	assert(status != "non initialized");

	if (status=="key initialized") {
		Generators=Map.submatrix(key);
		vector< Integer > help(dim);
		Support_Hyperplanes=Invert(Generators, help, volume); //test for arithmetic
		//overflow performed
		volume=Iabs(volume);
		diagonal=v_abs(help);
		Support_Hyperplanes=Support_Hyperplanes.transpose();
		multiplicators=Support_Hyperplanes.make_prime();
		Hilbert_Basis=list< vector<Integer> >();
		Homogeneous_Elements=list< vector<Integer> >();
		H_Vector=vector<Integer>(dim,0);
		status="initialized";
	}
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::hilbert_basis_interior(){
	assert(status == "initialized");

	//transformation

	int i,k,last;
	vector < Integer > norm(1);
	vector<Integer> point(dim,0);
	set < vector<Integer> > Candidates;
	typename set <vector <Integer> >::iterator c;
	//generating vector e=b_1*u_1+...+b_n*u_n (see documentation)
	while (1) {
		last=-1;
		for (i = 0; i < dim; i++) {
			if (point[i]<diagonal[i]-1) {
				last=i;
			}
		}
		if (last==-1) {
			break;
		}
		point[last]++;
		for (i = last+1; i <dim; i++) {
			point[i]=0;
		}
		vector<Integer> new_element=Support_Hyperplanes.MxV(point);
		for (k = 0; k < dim; k++) {
			new_element[k]= new_element[k]*multiplicators[k];
		}
		v_reduction_modulo(new_element,volume);
		norm[0]=0;
		for (k = 0; k < dim; k++) {
			norm[0]+=new_element[k];
		}
		new_element=v_merge(norm,new_element);
		Candidates.insert(new_element);
	}
	c=Candidates.begin();
	while(c != Candidates.end()) {
		reduce_and_insert_interior((*c));
		Candidates.erase(c);
		c=Candidates.begin();
	}

	//inverse transformation
	//some test for arithmetic overflow may be implemented here

	l_cut_front(Hilbert_Basis,dim);
	typename list< vector<Integer> >::iterator j;
	for (j =Hilbert_Basis.begin(); j != Hilbert_Basis.end(); j++) {
		*j=Generators.VxM(*j);
		v_scalar_division(*j,volume);
	}
	status="Hilbert Basis interior calculated.";
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::hilbert_basis_interior(const Matrix<Integer>& Map){
	if (status!="hilbert basis calculated") {
		assert(status != "non initialized");

		if (status=="initialized") {
			hilbert_basis_interior();
			return;
		}
		if (status=="key initialized") {
			Generators=Map.submatrix(key);
			vector< Integer > help(dim);
			Support_Hyperplanes=Invert(Generators, help, volume); //test for arithmetic
			//overflow performed
			volume=Iabs(volume);
			diagonal=v_abs(help);
			Support_Hyperplanes=Support_Hyperplanes.transpose();
			multiplicators=Support_Hyperplanes.make_prime();
			list< vector<Integer> >  Help;
			Hilbert_Basis=Help;
			status="initialized";
			hilbert_basis_interior();
			return;
		}
	}
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::hilbert_basis_interior_h_vector(const vector<Integer>& Form){
	assert(status == "initialized");

	//transformation
	vector < Integer > norm(1);
	set < vector<Integer> > Candidates;
	typename set <vector <Integer> >::iterator c;
	int i,k,last,h;
	int counter;
	Integer to_int,hom;
	vector<Integer> point(dim,0);
	vector<Integer> Help(dim);
	while (1){
		last=-1;
		for (i = 0; i < dim; i++) {
			if (point[i]<diagonal[i]-1) {
				last=i;
			}
		}
		if (last==-1) {
			break;
		}
		point[last]++;
		for (i = last+1; i <dim; i++) {
			point[i]=0;
		}
		vector<Integer> new_element=Support_Hyperplanes.MxV(point);
		for (k = 0; k < dim; k++) {
			new_element[k]= new_element[k]*multiplicators[k];
		}
		v_reduction_modulo(new_element,volume);
		//counting h-vector
		vector< int > Face(dim,0);
		for (k = 0; k <New_Face.size(); k++) {
			Face[New_Face[k]-1]=1;
		}
		for (k = 0; k <dim; k++) {
			if (new_element[k]!=0) {
				Face[k]=1;
			}
		}
		counter=0;
		for (k = 0; k <dim; k++) {
			if (Face[k]) {
				counter++;
			}
		}
		Help=Generators.VxM(new_element);
		v_scalar_division(Help,volume);
		hom=v_scalar_product(Help,Form);
		if (hom==1) {
			Homogeneous_Elements.push_back(Help);
		}
		to_int=counter-hom;
		h=explicit_cast_to_long(to_int);        //explicit cast used here for speed
		//the result of the above operation should be no greater than dim
		H_Vector[h]++;

		//preparing for reduction

		norm[0]=0;
		for (k = 0; k < dim; k++) {
			norm[0]+=new_element[k];
		}
		new_element=v_merge(norm,new_element);
		Candidates.insert(new_element);
	}
	c=Candidates.begin();
	while(c != Candidates.end()) {
		reduce_and_insert_interior((*c));
		Candidates.erase(c);
		c=Candidates.begin();
	}

	//inverse transformation
	//some test for arithmetic overflow may be implemented here

	l_cut_front(Hilbert_Basis,dim);
	typename list< vector<Integer> >::iterator j;
	for (j =Hilbert_Basis.begin(); j != Hilbert_Basis.end(); j++) {
		*j=Generators.VxM(*j);
		v_scalar_division(*j,volume);
	}

	status="Hilbert Basis interior and h vector calculated.";
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::hilbert_basis_interior_h_vector(const Matrix<Integer>& Map, const vector<Integer>& Form){
	assert(status=="key initialized");

	Generators=Map.submatrix(key);
	vector< Integer > help(dim);
	Support_Hyperplanes=Invert(Generators, help, volume); //test for arithmetic
	//overflow performed
	volume=Iabs(volume);
	diagonal=v_abs(help);
	Support_Hyperplanes=Support_Hyperplanes.transpose();
	multiplicators=Support_Hyperplanes.make_prime();
	list< vector<Integer> >  Help;
	Hilbert_Basis=Help;
	list< vector<Integer> >  Help1;
	Homogeneous_Elements=Help1;
	vector<Integer> Help2(dim,0);
	H_Vector=Help2;
	status="initialized";
	int i,j;
	for (i = 0; i <New_Face.size(); i++) {
		for ( j = 0; j <dim; j++) {
			if (New_Face[i]==key[j]) {
				New_Face[i]=j+1;
			}
		}
	}
	hilbert_basis_interior_h_vector(Form);
	return;
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::ht1_elements(const vector<Integer>& Form){
	assert(status == "initialized");

	//computing ht1 elements of the simplex
	int i,k,last;
	Integer hom;
	vector<Integer> point(dim,0);
	vector<Integer> Help(dim);
	while (1){
		last=-1;
		for (i = 0; i < dim; i++) {
			if (point[i]<diagonal[i]-1) {
				last=i;
			}
		}
		if (last==-1) {
			break;
		}
		point[last]++;
		for (i = last+1; i <dim; i++) {
			point[i]=0;
		}
		vector<Integer> new_element=Support_Hyperplanes.MxV(point);
		for (k = 0; k < dim; k++) {
			new_element[k]= new_element[k]*multiplicators[k];
		}
		v_reduction_modulo(new_element,volume);

		Help=Generators.VxM(new_element);
		v_scalar_division(Help,volume);
		hom=v_scalar_product(Help,Form);
		if (hom==1) {
			Homogeneous_Elements.push_back(Help);
		}
	}
	status="ht1 elements calculated.";
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::h_vector(const vector<Integer>& Form){
	assert(status == "initialized");

	//computing h-vector for the simplex

	int i,k,last,h;
	int counter;
	Integer to_int,hom;
	vector<Integer> point(dim,0);
	vector<Integer> Help(dim);
	while (1){
		last=-1;
		for (i = 0; i < dim; i++) {
			if (point[i]<diagonal[i]-1) {
				last=i;
			}
		}
		if (last==-1) {
			break;
		}
		point[last]++;
		for (i = last+1; i <dim; i++) {
			point[i]=0;
		}
		vector<Integer> new_element=Support_Hyperplanes.MxV(point);
		for (k = 0; k < dim; k++) {
			new_element[k]= new_element[k]*multiplicators[k];
		}
		v_reduction_modulo(new_element,volume);
		//counting h-vector
		vector< int > Face(dim,0);
		for (k = 0; k <New_Face.size(); k++) {
			Face[New_Face[k]-1]=1;
		}
		for (k = 0; k <dim; k++) {
			if (new_element[k]!=0) {
				Face[k]=1;
			}
		}
		counter=0;
		for (k = 0; k <dim; k++) {
			if (Face[k]) {
				counter++;
			}
		}
		Help=Generators.VxM(new_element);
		v_scalar_division(Help,volume);
		hom=v_scalar_product(Help,Form);
		if (hom==1) {
			Homogeneous_Elements.push_back(Help);
		}
		to_int=counter-hom;
		h=explicit_cast_to_long(to_int);        //explicit cast used here for speed
		//the result of the above operation should be no greater than dim
		H_Vector[h]++;
	}
	status="h vector calculated.";
}

//---------------------------------------------------------------------------

template<typename Integer>
void Simplex<Integer>::h_vector(const Matrix<Integer>& Map, const vector<Integer>& Form){
	assert(status == "key initialized");

	Generators=Map.submatrix(key);
	vector< Integer > help(dim);
	Support_Hyperplanes=Invert(Generators, help, volume); //test for arithmetic
	//overflow performed
	volume=Iabs(volume);
	diagonal=v_abs(help);
	Support_Hyperplanes=Support_Hyperplanes.transpose();
	multiplicators=Support_Hyperplanes.make_prime();
	list< vector<Integer> >  Help1;
	Homogeneous_Elements=Help1;
	vector<Integer> Help2(dim,0);
	H_Vector=Help2;
	status="initialized";
	int i,j;
	for (i = 0; i <New_Face.size(); i++) {
		for ( j = 0; j <dim; j++) {
			if (New_Face[i]==key[j]) {
				New_Face[i]=j+1;
			}
		}
	}
	h_vector(Form);
	return;
}


} /* end namespace */
