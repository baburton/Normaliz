/*
 * Normaliz
 * Copyright (C) 2007-2014  Winfried Bruns, Bogdan Ichim, Christof Soeger
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
 * As an exception, when this program is distributed through (i) the App Store
 * by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or (iii) Google Play
 * by Google Inc., then that store may impose any digital rights management,
 * device limits and/or redistribution restrictions that are required by its
 * terms of service.
 */

/**
 * The class Sublattice_Representation represents a sublattice of Z^n as Z^r.
 * To transform vectors of the sublattice  use:
 *    Z^r --> Z^n    and    Z^n -->  Z^r
 *     v  |-> vA             u  |-> (uB)/c
 * A  r x n matrix
 * B  n x r matrix
 * c  Integer
 */


#include "libnormaliz/sublattice_representation.h"
#include "libnormaliz/vector_operations.h"

//---------------------------------------------------------------------------

namespace libnormaliz {
using namespace std;

/**
 * creates a representation of Z^n as a sublattice of itself
 */
template<typename Integer>
Sublattice_Representation<Integer>::Sublattice_Representation(size_t n) {
    dim = n;
    rank = n;
    external_index = 1;
    A = Matrix<Integer>(n);
    B = Matrix<Integer>(n);
    c = 1;
    Equations_computed=false;
    Congruences_computed=false;
    is_identity=true;
}

//---------------------------------------------------------------------------

/**
 * Main Constructor
 * creates a representation of a sublattice of Z^n
 * if direct_summand is false the sublattice is generated by the rows of M
 * otherwise it is a direct summand of Z^n containing the rows of M
 */
 
 template<typename Integer>
Sublattice_Representation<Integer>::Sublattice_Representation(const Matrix<Integer>& M, bool take_saturation) {
    bool success;
    initialize(M,take_saturation,success);
    if(!success){
        Matrix<mpz_class> mpz_M(M.nr,M.nc);
        mat_to_mpz(M,mpz_M);
        Sublattice_Representation<mpz_class> mpz_SLR;
        mpz_SLR.initialize(mpz_M,take_saturation,success);
        A=Matrix<Integer>(mpz_SLR.A.nr,mpz_SLR.A.nc);
        B=Matrix<Integer>(mpz_SLR.B.nr,mpz_SLR.B.nc);
        mat_to_Int(mpz_SLR.A,A);
        mat_to_Int(mpz_SLR.B,B);
        convert(c, mpz_SLR.c);
        rank=mpz_SLR.rank;        
    }
}


template<typename Integer>
void Sublattice_Representation<Integer>::initialize(const Matrix<Integer>& M, bool take_saturation, bool& success) {

    Equations_computed=false;
    Congruences_computed=false;
    is_identity=false;

    success=true;

    dim=M.nr_of_columns();
    Matrix<Integer> N=M;    

    rank=N.row_echelon_reduce(success);  // reduce is importnat here, will be used
    if(!success)
        return;

    if(rank==dim && take_saturation){
        A = B = Matrix<Integer>(dim);
        c=1;
        is_identity=true;
        return;   
    }

    mpz_class row_index=1;  // product of the corner elements in the row echelon form
    vector<key_t> col(rank);
    vector<bool> col_is_corner(dim,false); // indicates whether the column is a corner in the 
    for(size_t k=0;k<rank;++k){            // row echelin form
        size_t j=0;
        for(;j<dim;++j)
            if(N[k][j]!=0)
                break;
        col_is_corner[j]=true;
        col[k]=j;
        if(N[k][j]<0)
            v_scalar_multiplication<Integer>(N[k],-1);  // make corner positive
        row_index*=convertTo<mpz_class>(N[k][j]);
    }
    
    if(row_index==1 && rank==dim){  // the sublattice is the full lattice and no saturation needed
        A = B = Matrix<Integer>(dim);
        c=1;
        is_identity=true;
        return;   
    }
    
    A=Matrix<Integer>(rank, dim);
    B=Matrix<Integer>(dim,rank);
    
    if(row_index==1){  // no saturation needed since sublattice is direct summand
    
        for(size_t k=0;k<rank;++k)
            A[k]=N[k];    // A is just the basis of our sublattice
        size_t j=0;
        for(size_t k=0;k<dim;++k){
            if(col_is_corner[k]){
                B[k][j]=1;  // projection to the corner columns, allowed because of reduction!
                j++;
            }
        };
        c=1;
        return;               
    }
    
    if(!take_saturation){
        Matrix<Integer> P(dim,dim);  // A augmented by unit vectors to full rank
        for(size_t k=0;k<rank;++k)
            A[k]=P[k]=N[k];
        size_t k=rank;
        for(size_t j=0;j<dim;++j){
            if(col_is_corner[j])
                continue;
            P[k][j]=1;
            k++;        
        }
        Matrix<Integer> Q=P.invert_unprotected(c,success);
        if(!success)
            return;
        
        for(k=0;k<dim;++k) // we take the partial inverse belonging to the first rankk rows of A
            for(size_t j=0;j<rank;++j)
                B[k][j]=Q[k][j];
        return;               
    }
    
    // now we must take the saturation.
    // We do it by computing a complement of the smallest direct summand containing 
    // of the sublattice and then taking its complement.
    
    Matrix<Integer> R_inv(dim);
    success=N.column_trigonalize(rank,R_inv);
    Matrix<Integer> R=R_inv.invert_unprotected(c,success);   // yields c=1 as it should be in this case
    if(!success)
        return;
    
    for (size_t i = 0; i < rank; i++) {
        for (size_t j = 0; j < dim; j++) {
                A[i][j]=R[i][j];
                B[j][i]=R_inv[j][i];
        }
    }
    return; 
}

//---------------------------------------------------------------------------
//                       Constructor by conversion
//---------------------------------------------------------------------------

template<typename Integer>
template<typename IntegerFC>
Sublattice_Representation<Integer>::Sublattice_Representation(const 
             Sublattice_Representation<IntegerFC>& Original) {
                 
    convert(A,Original.A);
    convert(B,Original.B);
    dim=Original.dim;
    rank=Original.rank;
    convert(c,Original.c);
    is_identity=Original.is_identity;
    Equations_computed=Original.Equations_computed;
    Congruences_computed=Original.Congruences_computed;
    convert(Equations,Original.Equations);
    convert(Congruences,Original.Congruences);
    external_index=Original.external_index;    
}


//---------------------------------------------------------------------------
//                       Manipulation operations
//---------------------------------------------------------------------------

/* first this then SR when going from Z^n to Z^r */
template<typename Integer>
void Sublattice_Representation<Integer>::compose(const Sublattice_Representation& SR) {
    assert(rank == SR.dim); //TODO vielleicht doch exception?
    
    if(SR.is_identity)
        return;
    
    if(is_identity){
        *this=SR;
        return;
    }        
    
    Equations_computed=false;
    Congruences_computed=false;

    rank = SR.rank;
    // A = SR.A * A
    A = SR.A.multiplication(A);
    // B = B * SR.B
    B = B.multiplication(SR.B);
    c = c * SR.c;
    
    //check if a factor can be extraced from B  //TODO necessary?
    Integer g = B.matrix_gcd();
    g = libnormaliz::gcd(g,c);  //TODO necessary??
    if (g > 1) {
        c /= g;
        B.scalar_division(g);
    }
    is_identity&=SR.is_identity;
}

template<typename Integer>
void Sublattice_Representation<Integer>::compose_dual(const Sublattice_Representation& SR) {

    assert(rank == SR.dim); //
    assert(SR.c==1);
    
    if(SR.is_identity)
        return;
    
    Equations_computed=false;
    Congruences_computed=false;    
    rank = SR.rank;
    
    if(is_identity){
        A=SR.B.transpose();
        B=SR.A.transpose();
        is_identity=false;
        return;
    }
    
    // Now we compose with the dual of SR
    A = SR.B.transpose().multiplication(A);
    // B = B * SR.B
    B = B.multiplication(SR.A.transpose());
    
    //check if a factor can be extraced from B  //TODO necessary?
    Integer g = B.matrix_gcd();
    g = libnormaliz::gcd(g,c);  //TODO necessary??
    if (g > 1) {
        c /= g;
        B.scalar_division(g);
    }
    is_identity&=SR.is_identity;
}

//---------------------------------------------------------------------------
//                       Transformations
//---------------------------------------------------------------------------

template<typename Integer>
Matrix<Integer> Sublattice_Representation<Integer>::to_sublattice (const Matrix<Integer>& M) const {
    Matrix<Integer> N;
    if(is_identity)
        N=M;
    else        
        N = M.multiplication(B);
    if (c!=1) N.scalar_division(c);
    return N;
}
template<typename Integer>
Matrix<Integer> Sublattice_Representation<Integer>::from_sublattice (const Matrix<Integer>& M) const {
    Matrix<Integer> N;
    if(is_identity)
        N=M;
    else        
        N = M.multiplication(A);
    return N;
}

template<typename Integer>
Matrix<Integer> Sublattice_Representation<Integer>::to_sublattice_dual (const Matrix<Integer>& M) const {
    Matrix<Integer> N;
    if(is_identity)
        N=M;
    else        
        N = M.multiplication(A.transpose());
    N.make_prime();
    return N;
}

template<typename Integer>
Matrix<Integer> Sublattice_Representation<Integer>::from_sublattice_dual (const Matrix<Integer>& M) const {
    Matrix<Integer> N;
    if(is_identity)
        N=M;
    else        
        N =  M.multiplication(B.transpose());
    N.make_prime();
    return N;
}


template<typename Integer>
vector<Integer> Sublattice_Representation<Integer>::to_sublattice (const vector<Integer>& V) const {
    if(is_identity)
        return V;
    vector<Integer> N = B.VxM(V);
    if (c!=1) v_scalar_division(N,c);
    return N;
}

template<typename Integer>
vector<Integer> Sublattice_Representation<Integer>::from_sublattice (const vector<Integer>& V) const {
    if(is_identity)
        return V;
    vector<Integer> N = A.VxM(V);
    return N;
}

template<typename Integer>
vector<Integer> Sublattice_Representation<Integer>::to_sublattice_dual (const vector<Integer>& V) const {
    vector<Integer> N;
    if(is_identity)
        N=V;
    else    
        N = A.MxV(V);
    v_make_prime(N);
    return N;
}

template<typename Integer>
vector<Integer> Sublattice_Representation<Integer>::from_sublattice_dual (const vector<Integer>& V) const {
    vector<Integer> N; 
    if(is_identity)
        N=V;
    else    
        N = B.MxV(V);
    v_make_prime(N);
    return N;
}

template<typename Integer>
vector<Integer> Sublattice_Representation<Integer>::to_sublattice_dual_no_div (const vector<Integer>& V) const {
    if(is_identity)
        return V;
    vector<Integer> N = A.MxV(V);
    return N;
}

//---------------------------------------------------------------------------
//                       Data access
//---------------------------------------------------------------------------

/* returns the dimension of the ambient space */
template<typename Integer>
size_t Sublattice_Representation<Integer>::getDim() const {
    return dim;
}

//---------------------------------------------------------------------------

/* returns the rank of the sublattice */
template<typename Integer>
size_t Sublattice_Representation<Integer>::getRank() const {
    return rank;
}

//---------------------------------------------------------------------------

template<typename Integer>
const Matrix<Integer>& Sublattice_Representation<Integer>::getEmbeddingMatrix() const {
    return A;
} 

template<typename Integer>
const vector<vector<Integer> >& Sublattice_Representation<Integer>::getEmbedding() const{
    return getEmbeddingMatrix().get_elements();
}

//---------------------------------------------------------------------------

template<typename Integer>
const Matrix<Integer>& Sublattice_Representation<Integer>::getProjectionMatrix() const {
    return B;
}

template<typename Integer>
const vector<vector<Integer> >& Sublattice_Representation<Integer>::getProjection() const{
    return getProjectionMatrix().get_elements();
}


//---------------------------------------------------------------------------

template<typename Integer>
Integer Sublattice_Representation<Integer>::getAnnihilator() const {
    return c;
}

//---------------------------------------------------------------------------

template<typename Integer>
bool Sublattice_Representation<Integer>::IsIdentity() const{ 
    return is_identity;
}

//---------------------------------------------------------------------------

/* returns the congruences defining the sublattice */

template<typename Integer>
const Matrix<Integer>& Sublattice_Representation<Integer>::getEquationsMatrix() const{

    if(!Equations_computed)
        make_equations();
    return Equations;
}

template<typename Integer>
const vector<vector<Integer> >& Sublattice_Representation<Integer>::getEquations() const{
        return getEquationsMatrix().get_elements();
}

template<typename Integer>
void Sublattice_Representation<Integer>::make_equations() const{

    if(rank==dim)
        Equations=Matrix<Integer>(0,dim);
    else
        Equations=A.kernel();    
    Equations_computed=true;
}

template<typename Integer>
const Matrix<Integer>& Sublattice_Representation<Integer>::getCongruencesMatrix() const{

    if(!Congruences_computed)
        make_congruences();
    return Congruences;
}

template<typename Integer>
const vector<vector<Integer> >& Sublattice_Representation<Integer>::getCongruences() const{
    return getCongruencesMatrix().get_elements();
}

template<typename Integer>
mpz_class Sublattice_Representation<Integer>::getExternalIndex() const{

    if(!Congruences_computed)
        make_congruences();
    return external_index;
}

template<typename Integer>
void Sublattice_Representation<Integer>::make_congruences() const {

    if ( c == 1) { // no congruences then
        Congruences=Matrix<Integer>(0,dim+1);
        Congruences_computed=true;
        external_index=1;
        return;
    }
    
    size_t dummy;
    Matrix<Integer> A_Copy=A;
    Matrix<Integer> Transf=A_Copy.SmithNormalForm(dummy);

    // Congruences given by first rank columns of Transf transposed and with an extra column for the modulus m
    // The moduli are the diagonal elements of the Smith normal form
    
    // Transf.pretty_print(cout);

    Transf.append(Matrix<Integer>(1,dim));
    Transf = Transf.transpose();
    Matrix<Integer> Transf2(0,dim+1); //only the relavant congruences
    for(size_t k=0;k<rank;++k){
        if(A_Copy[k][k]!=1){
            Transf2.append(Transf[k]);
            Transf2[Transf2.nr-1][dim]=A_Copy[k][k];
            for(size_t j=0;j<dim;++j){
                Transf2[Transf2.nr-1][j]%=A_Copy[k][k];
                if(Transf2[Transf2.nr-1][j]<0)
                    Transf2[Transf2.nr-1][j]+=A_Copy[k][k];
            }
        
        }   
    }
    Congruences=Transf2;
    Congruences_computed=true;
    external_index=1;
    for(size_t i=0;i<Transf2.nr;++i)
        external_index*=convertTo<mpz_class>(Transf2[i][dim]);
}

}
