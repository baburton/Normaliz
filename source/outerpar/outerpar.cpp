#include <stdlib.h>
#include <vector>
#include <fstream>
#include <string>
#ifdef _OPENMP
#include <omp.h>
#endif
using namespace std;

#include "libnormaliz/libnormaliz.h"
#include "libnormaliz/cone.h"
#include "libnormaliz/vector_operations.h"
#include "libnormaliz/cone_property.h"
#include "libnormaliz/integer.h"
#include "libnormaliz/matrix.h"
using namespace libnormaliz;

typedef long long Integer;



int main(int argc, char* argv[]){

    Matrix<Integer> Gens=readMatrix<Integer>(string("small_gens.mat"));
    
    vector<Cone<Integer> > ParCones(16);
    #pragma omp parallel for
    for(size_t i=0;i<ParCones.size();++i){
        ParCones[i]=Cone<Integer>(Type::cone,Gens);
        // ParCones[i].setVerbose(true);
        switch(i%8){
            case 0: ParCones[i].compute(ConeProperty::DefaultMode);
                break;
            case 1: ParCones[i].compute(ConeProperty::DualMode, ConeProperty::Deg1Elements);
                break;
            case 2: ParCones[i].compute(ConeProperty::Projection);
                break;            
            case 3: ParCones[i].compute(ConeProperty::ProjectionFloat);
                break;
            case 4: ParCones[i].compute(ConeProperty::Approximate, ConeProperty::IsGorenstein);
                break;
            case 5: ParCones[i].compute(ConeProperty::SupportHyperplanes);
                break;
            case 6: ParCones[i].compute(ConeProperty::IntegerHull);
                break;
            case 7: ParCones[i].compute(ConeProperty::IsIntegrallyClosed);
                break;
            default: break;
        }        
    }

}  //end main