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

tipoHazard detectaHazardDados(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM){

    uint8_t destinoEX;

    if(ID_EX->sinais.RegDst)
        destinoEX = ID_EX->rd;
    else
        destinoEX = ID_EX->rt;

    int escMEM = (EX_MEM->sinais.EscReg == 1) && (EX_MEM->rd != 0);

int hazard_EX = ID_EX->sinais.EscReg && destinoEX != 0 && (destinoEX == IF_ID->inst.rs || destinoEX == IF_ID->inst.rt);
    
     int hazard_MEM = escMEM && (EX_MEM->rd == IF_ID->inst.rs || EX_MEM->rd == IF_ID->inst.rt);

    if(hazard_EX || hazard_MEM){
        return 1;
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