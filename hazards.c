#include <stdio.h>
#include "minimips.h"
#include "hazards.h"

tipoHazard unidadeDetecHazards(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM){
    
    int hazardDado=0, hazardControle=0;

    hazardDado = detectaHazardDados(IF_ID, ID_EX, EX_MEM);    
    hazardControle = detectaHazardControle(ID_EX, EX_MEM);

    if(hazardDado==1){
        return 1;
    }
    
    if(hazardControle==1){
        return 2;
    }

    return 0;
}
tipoHazard detectaHazardDados(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM) {

    (void)EX_MEM;
    int instrucaoAnteriorEhLW = (ID_EX->opcode == 11); 

    if (instrucaoAnteriorEhLW) {
        if (ID_EX->rt != 0 && (ID_EX->rt == IF_ID->inst.rs || ID_EX->rt == IF_ID->inst.rt)) {
            return 1; 
        }
    }

    return 0;
}

tipoHazard detectaHazardControle(ID_EX *ID_EX, EX_MEM *EX_MEM){
    if(ID_EX->sinais.jump==1){
        return 1;        
    }
    else if(EX_MEM->sinais.branch==1 && EX_MEM->zero==1){
        return 1;
    }
    
    return 0;
}