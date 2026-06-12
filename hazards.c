#include <stdio.h>
#include "hazards.h"

tipoHazard unidadeDetecHazards(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM){
    int hazardDado = 0, hazardControle = 0;

    hazardDado = detectaHazardDados(IF_ID, ID_EX, EX_MEM);
    hazardControle = detectaHazardControle(ID_EX, EX_MEM);

    if(hazardDado == 1){
        return 1; // Retorna Stall para Hazard de Dados
    }

    if(hazardControle == 1){
        return 2; // Retorna Flush para Hazard de Controle
    }

    return 0; // Sem hazards
}

tipoHazard detectaHazardDados(IF_ID *IF_ID, ID_EX *ID_EX, EX_MEM *EX_MEM){

    int escEX  = (ID_EX->sinais.EscReg == 1)  && (ID_EX->rd != 0);
    int escMEM = (EX_MEM->sinais.EscReg == 1) && (EX_MEM->rd != 0);

    // Verifica dependência de dados entre a instrução em ID (IF_ID) e as anteriores em EX ou MEM
    int hazard_EX  = escEX  && (ID_EX->rd == IF_ID->inst.rs  || ID_EX->rd == IF_ID->inst.rt);
    int hazard_MEM = escMEM && (EX_MEM->rd == IF_ID->inst.rs || EX_MEM->rd == IF_ID->inst.rt);

    if(hazard_EX || hazard_MEM){
        return 1;
    }

    return 0;
}

tipoHazard detectaHazardControle(ID_EX *ID_EX, EX_MEM *EX_MEM){
    // Jumps são resolvidos no estágio ID
    if(ID_EX->sinais.jump == 1){
        return 1;
    }
    // Branches (BEQ) são resolvidos após a ULA no estágio EX (refletido em EX_MEM)
    else if(EX_MEM->sinais.branch == 1 && EX_MEM->zero == 1){
        return 1;
    }

    return 0;
}
