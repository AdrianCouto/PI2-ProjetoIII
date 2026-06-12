#include <stdio.h>
#include "hazards.h"

tipoHazard unidadeDetecHazards(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM){
    int hazardDado=0, hazardControle=0;

    hazardDado = detectaHazardDados(IF_ID, ID_EX, EX_MEM);
    

    hazardControle = detectaHazardControle(IF_ID);

}

tipoHazard detectaHazardDados(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM){
 
    if((IF_ID->inst.rs==ID_EX->rd && ID_EX->sinais.EscReg==1) || (IF_ID->inst.rs==EX_MEM->rd && EX_MEM->sinais.EscReg==1) || (IF_ID->inst.rt==ID_EX->rd && ID_EX->sinais.EscReg==1) || (IF_ID->inst.rt==EX_MEM->rd && EX_MEM->sinais.EscReg)){  // Analisar como mudar esse if gigante
        printf("\nHazard de dados detectado!\n");
        return 1;
    }
    
    return 0;
}