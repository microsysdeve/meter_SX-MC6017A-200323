/*
*********************************************************************************************************
*                                              HT60XX
*                                           Library Function
*
*                                   Copyright 2013, Hi-Trend Tech, Corp.
*                                        All Rights Reserved
*                                         
*
* Project      : HT60xx
* File         : example.c
* By           : SocTeam
* Version      : CurrentVersion
* Description  : 
*********************************************************************************************************
*/

#define  __AES_EXAMPLE_C
//#undef   __AES_EXAMPLE_C

#ifdef   __AES_EXAMPLE_C

#include "aes_example.h"
#include "ht6xxx_lib.h"

#if defined HT6x2x  ||  defined  HT501x             /* This File Only support HT6x2x and HT501x */ 

/*
*********************************************************************************************************
*                                           ���غ�/�ṹ��
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             ���ر���
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           ���غ�������
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                GHASH H������ʼ������
*
* ����˵��: H������ʼ������
*						�㷨��	H=E(K,0[128]),    E()Ϊaes���ܣ�0[128]Ϊ128λ'0'��KΪkey��Կ
* ��ڲ���: 
*						AESKeyMode  		AESKeyMode_TypeDef�ͱ���������ģʽ
*														KEY_Mode_128bits
*														KEY_Mode_192bits
*														KEY_Mode_256bits
*						pKeyAddr		  	uint32_t *�ͱ�������ָ�����洢��Կ
*						pOutDataAddr  	uint32_t *�ͱ�������ָ�����洢H��ʼֵ: pIntDataAddr[0]Ϊ��λ
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void GCM_GhashHeader_init(AESKeyMode_TypeDef AESKeyMode,uint32_t *pKeyAddr,uint32_t *pOutDataAddr)
{
		uint8_t i;
		Parameter128bits_TypeDef	Aes_ghash_temp;										/*!< ����128bits����ʱ����				*/
		
		for(i=0;i<4;i++)
		{
				Aes_ghash_temp.u32[i]=0x0000;															/*!< ��ʱ����	��ֵ0x0000				*/
		}
		HT_AES_Encrypt(AESKeyMode,pKeyAddr,Aes_ghash_temp.u32);			/*!< E(K,0[128])��aes��������			*/
		
		while(HT_AES_Busy_StatusGet());															/*!< AES���ܽ����ж�							*/

		HT_AES_Read(pOutDataAddr);																	/*!< ����������д��	*pOutDataAddr	*/
}

/*
*********************************************************************************************************
*                                GHASH��H,A,C) ��֤��������
*
* ����˵��: GHASH��H,A,C) ��֤��������,
*						�㷨��	GHASH(H,A,C)	= Xm+n+1,    Xm+n+1Ϊghash��֤�����mΪA�ķֶ�����128bitsΪ1�Σ���nΪc�ķֶ���,AΪauth data����֤���ݣ���CΪciphertext�����ģ�
*										Xi = 	0																,for i=0		
*											 =	(Xi-1 ^ Ai) . H									,for i=1,...,m-1
*											 =	(Xm-1 ^ Am||0[128-v]) . H				,for i=m												
*											 =	(Xi-1 ^ Ci-m) . H								,for i=m+1,...,m+n-1
*											 =	(Xm+n-1 ^ Cn||0[128-u]) . H			,for i=m+n
*											 =	(Xm+n	^ (len(A)||len(C))) . H		,for i=m+n+1
*													ע:Am||0[128-v]��ΪA���ֶβ���128bits���㣬vΪ���ֶ�λ��
*														 Cn||0[128-u]��ΪC���ֶβ���128bits���㣬uΪ���ֶ�λ��
*														 len(A),����һ��64-bits �ַ�������ʾ����A��λ����bit��
* ��ڲ���: 
*						GhashHeaderAddr		uint32_t *�ͱ�������ָ�����洢GhashHeader��ֵ
*						pADataAddr  			uint8_t *�ͱ�������ָ�����洢Aֵ
*						LenAData					uint64_t �ͱ�������ʾAֵ���ȣ���λbyte��
*						*pCtextAddr  			uint8_t *�ͱ�������ָ�����洢Cֵ
*						LenCtext					uint64_t �ͱ�������ʾCֵ���ȣ���λbyte��
*						OutData						Parameter128bits_TypeDef*�����z��ָ�����洢ghash����������
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void GCM_GhashAuth(uint32_t *GhashHeaderAddr,uint8_t *pADataAddr,uint64_t LenAData,uint8_t *pCtextAddr,uint64_t LenCtext,Parameter128bits_TypeDef* OutData)
{
		uint8_t i; 
		Parameter128bits_TypeDef	Aes_ghash_temp,LenData_temp;
	
		for(i=0;i<4;i++)
		{
				OutData->u32[i]=0x0000;																/*!< OutData����	��ֵ0x0000				*/
		}
	
		/*(len(A)||len(C))����  */
		LenData_temp.u64[0] = LenCtext * 8;												/*!<(len(A)||len(C))����������		*/
		LenData_temp.u64[1] = LenAData * 8;
	
		/*Ai���㲿��  */
		for(i=0;i<4;i++)
		{
				OutData->u32[i]=0x0000;																/*!< OutData����	��ֵ0x0000				*/
				Aes_ghash_temp.u32[i]=0x0000;													/*!< ��ʱ����	��ֵ0x0000						*/
		}
		//(Xi-1 ^ Ai) . H							,for i=1,...,m-1
		while(LenAData>=16)																				/*!< Aλ���ж�	16bytes��128bits��	*/
		{
				for(i=0;i<16;i++)
				{
						OutData->u8[i] ^= *pADataAddr++;										/*!< (Xi-1 ^ Ai), 128bits ������	*/
				}
				LenAData -= 16;																				/*!< �¸�128bit�ֶ�									*/
				HT_GHASH_Multiply(OutData->u32, GhashHeaderAddr);			/*!< ��OutData . H������						*/
				while(HT_GHASH_Busy_StatusGet());											/*!< Ghash�˷���������ж�					*/
				HT_GHASH_Multiply_Read(OutData->u32);									/*!< ��ȡ���������洢��	OutData		*/
		}
		//(Xm-1 ^ Am||0[128-v]) . H			,for i=m
		if(LenAData)
		{
				for(i=0;i<4;i++)
				{
						Aes_ghash_temp.u32[i]=0x0000;											/*!< ��ʱ����	��ֵ0x0000						*/
				}
				for(i=16-LenAData;i<16;i++)
				{
						Aes_ghash_temp.u8[i] = *pADataAddr++;							/*!< ����128bits��AData�͵�Aes_ghash_temp��λ	����λ����	*/
				}
				HT_AES_Xor128bits(OutData->u32,Aes_ghash_temp.u32, OutData->u32);
																															/*!< ������AData������OutData������㣬�������OutData	*/
				HT_GHASH_Multiply(OutData->u32, GhashHeaderAddr);			/*!< ��OutData . H������						*/
				while(HT_GHASH_Busy_StatusGet());											/*!< Ghash�˷���������ж�					*/
				HT_GHASH_Multiply_Read(OutData->u32);									/*!< ��ȡ���������洢��	OutData		*/
		}	
		
	/*Ci���㲿��*/
		//(Xi-1 ^ Ci-m) . H								,for i=m+1,...,m+n-1
		while(LenCtext>=16)																				/*!< Cλ���ж�	16bytes��128bits��	*/
		{
				for(i=0;i<16;i++)
				{
						OutData->u8[i] ^= *pCtextAddr++;										/*!< (Xi-1 ^ Ci-m), 128bits ������	*/
				}
				LenCtext -= 16;																				/*!< �¸�128bit�ֶ�									*/
				HT_GHASH_Multiply(OutData->u32, GhashHeaderAddr);			/*!< ��OutData . H������						*/
				while(HT_GHASH_Busy_StatusGet());											/*!< Ghash�˷���������ж�					*/
				HT_GHASH_Multiply_Read(OutData->u32);									/*!< ��ȡ���������洢��	OutData		*/
		}
		//(Xm+n-1 ^ Cn||0[128-u]) . H			,for i=m+n
		if(LenCtext)
		{
				for(i=0;i<4;i++)
				{
						Aes_ghash_temp.u32[i]=0x0000;											/*!< ��ʱ����	��ֵ0x0000						*/
				}
				for(i=16-LenCtext;i<16;i++)
				{
						Aes_ghash_temp.u8[i] = *pCtextAddr++;							/*!< ����128bits��CData�͵�Aes_ghash_temp��λ	����λ����	*/
				}
				HT_AES_Xor128bits(OutData->u32,Aes_ghash_temp.u32, OutData->u32);
																															/*!< ������AData������OutData������㣬�������OutData	*/
				HT_GHASH_Multiply(OutData->u32, GhashHeaderAddr);			/*!< ��OutData . H������						*/
				while(HT_GHASH_Busy_StatusGet());											/*!< Ghash�˷���������ж�					*/
				HT_GHASH_Multiply_Read(OutData->u32);									/*!< ��ȡ���������洢��	OutData		*/
		}	
		
	/*������㲿��*/
		//(Xm+n	^ (len(A)||len(C))) . H		,for i=m+n+1
		HT_AES_Xor128bits(OutData->u32,LenData_temp.u32, OutData->u32);	
																															/*!< (len(A)||len(C))��OutData������㣬�������OutData	*/
		HT_GHASH_Multiply(OutData->u32, GhashHeaderAddr);					/*!< ��OutData . H������						*/
		while(HT_GHASH_Busy_StatusGet());													/*!< Ghash�˷���������ж�					*/
		HT_GHASH_Multiply_Read(OutData->u32);											/*!< ��ȡ���������洢��	OutData		*/	
}

/*
*********************************************************************************************************
*                                GCM Counter0(Y0)������ʼ������
*
* ����˵��: Counter0(Y0)������ʼ������
*						�㷨��Y0 = IV || 0[31]1   ,if len(IV)=96 bits(12 bytes)
*										 = GHASH(H,{},IV)	,otherwise
*									IVΪ��ʼ������0[31]1Ϊ31λ'0'��1λ'1',GHASH(H,{},IV)Ϊghash���㺯��
* ��ڲ���: 
*						pInitVectorAddr  	uint8_t *�ͱ�������ָ�����洢IV��ʼֵ: pInitVectorAddr[0]Ϊ��λ
*						LenIV							uint64_t	��ʼ����λ������λbyte
*						GhashHeaderAddr		uint32_t *�ͱ�������ָ�����洢GhashHeader��H��ֵ
*						Counter0					Parameter128bits_TypeDef*��������ָ�����洢ICounter0(y0)��ʼ�����
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void GCM_Counter0_init(uint8_t *pInitVectorAddr,uint64_t LenIV,uint32_t *GhashHeaderAddr,Parameter128bits_TypeDef* Counter0)
{
		uint8_t i;
		uint8_t	*ptemp;																							/*!< ��ʱָ�����								*/															
		
		for(i=0;i<4;i++)
		{
				Counter0->u32[i] = 0x0000;																/*!< counter0	��ֵ0x0000				*/
		}
		
		//Y0 = IV || 0[31]1				,if len(IV)=96 bits(12 bytes)
		if(LenIV == 12)																							/*!< IV����12 bytes	(96 bits)		*/
		{
				for(i=4;i<16;i++)
				{
						Counter0->u8[i] = *pInitVectorAddr++;								/*!< IV�͵�Counter0��96λ				*/
				}
				Counter0->u32[0]=0x0001;																	/*!< Counter0��32λ	��0��1			 */
		}
		
		//Y0 = GHASH(H,{},IV)			,otherwise
		else
		{
				GCM_GhashAuth(GhashHeaderAddr,ptemp,0x0,pInitVectorAddr,LenIV,Counter0);
																																/*!< Ghash��֤���㣬GHASH(H,{},IV)	 */
		}
}

/*
*********************************************************************************************************
*                                GCM Counter(Yi)�ۼӼ���
*
* ����˵��: Counter(Yi)�ۼӼ���
*						�㷨��Yi = Yi-1 + 1   ,i=1,2...n
*								Yi���ۼӼ��㣬�������ұ�32bits��Ч
*									
* ��ڲ���: 
*						Counter					Parameter128bits_TypeDef*��������ָ�����洢ICounter(Yi)�ۼӵ�counter����
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void GCM_Counter_increase(Parameter128bits_TypeDef* Counter)
{
		Counter->u32[0]++;													/*!< Yi = Yi-1 + 1	�����ұ�32bits��Ч			*/
}

/*
*********************************************************************************************************
*                                GCM_CTR����/���ܼ���
*
* ����˵��: GCM_CTR����/���ܼ���
*						�����㷨��Ci = Pi ^ E(K,Yi)   				,for i=1,2,...,n-1
*											 		 Pn ^ MSBu(E(K,Yn))			,for i=n
*						�����㷨��Pi = Ci ^ E(K,Yi)   				,for i=1,2,...,n-1
*											 		 Cn ^ MSBu(E(K,Yn))			,for i=n
*									PiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n-1)*128+u bits��1<= u <=128
*									CiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n-1)*128+u bits��1<= u <=128
*									E(K,Yi)Ϊ��Yi����AES��������
*									MSBu(S)Ϊ����1�ַ������ַ�������S���ݵ������u��bits
* ��ڲ���: 
*						pInttextAddr  	uint8_t *�ͱ�������ָ�����洢����/��������:
*						LenInttext			uint64_t	����/��������λ������λbyte
*						Counter0				Parameter128bits_TypeDef*��������ָ�����洢Counter0(y0)��ֵ
*						AESKeyMode			ö�ٱ�����ѡ�����ģʽ
*														KEY_Mode_128bits
*														KEY_Mode_192bits
*														KEY_Mode_256bits
*						pKeyAddr  			uint32_t *�ͱ�������ָ�����洢AES key : pKeyAddr[0]Ϊ��λ
*						pOuttextAddr		uint8_t *�ͱ�������ָ�����洢��/�����������
*
* ���ز���: ��                                      
* 
* ����˵��: ���ܲ���ʱ��pInttextAddr��LenInttextΪ�������룬pOuttextAddrΪ�������
*						���ܲ���ʱ��pInttextAddr��LenInttextΪ�������룬pOuttextAddrΪ�������
*********************************************************************************************************
*/
void GCM_CTR_Encrypt(uint8_t *pInttextAddr,uint64_t LenInttext,Parameter128bits_TypeDef* Counter0,AESKeyMode_TypeDef AESKeyMode, uint32_t *pKeyAddr,uint8_t *pOuttextAddr)
{
		uint8_t i; 
		Parameter128bits_TypeDef	Counter_temp,Aes_GCM_temp;
	
		for(i=0;i<4;i++)
		{
				Counter_temp.u32[i] = Counter0->u32[i];								/*!< counter0�浽Counter_temp����		*/
		}
	//Ci = Pi ^ E(K,Yi)   				,for i=1,2,...,n-1
		while(LenInttext>=16)																				/*!< Pλ���ж�	16bytes��128bits��	*/
		{
				GCM_Counter_increase(&Counter_temp);										/*!< GCM Counter(Yi)�ۼӼ���				*/
				HT_AES_Encrypt(AESKeyMode,pKeyAddr,Counter_temp.u32);
																															/*!< ��Counter(Yi)AES���ܲ���	�� E(K,Yi) 			*/
				while(HT_AES_Busy_StatusGet());												/*!< AES������������ж�						*/
				HT_AES_Read(Aes_GCM_temp.u32);												/*!< ��ȡ���������洢��	Aes_GCM_temp		*/
				for(i=0;i<16;i++)
				{
						*pOuttextAddr++ = Aes_GCM_temp.u8[i] ^ *pInttextAddr++;	
																															/*!< Pi ^ E(K,Yi)����								*/
				}
				LenInttext -= 16;																				/*!< �¸�128bit�ֶ�									*/			
		}
	//Cn = Pn ^ MSBu(E(K,Yn))			,for i=n
		if(LenInttext)
		{
				GCM_Counter_increase(&Counter_temp);										/*!< GCM Counter(Yi)�ۼӼ���				*/
				HT_AES_Encrypt(AESKeyMode,pKeyAddr,Counter_temp.u32);
																															/*!< ��Counter(Yn)AES���ܲ���	�� E(K,Yn) 	*/
				while(HT_AES_Busy_StatusGet());												/*!< AES������������ж�						*/
				HT_AES_Read(Aes_GCM_temp.u32);												/*!< ��ȡ���������洢��	Aes_GCM_temp1		*/
				for(i=16-LenInttext;i<16;i++)
				{
						*pOuttextAddr++ = Aes_GCM_temp.u8[i] ^ *pInttextAddr++;							
																															/*!< Pn ^ MSBu(E(K,Yn))����					*/
				}
		}		
}

/*
*********************************************************************************************************
*                                GCM ��֤��ǩ����
*
* ����˵��: GCM ��֤��ǩ����
*						�����㷨��T = MSBt(GHASH(H,A,C) ^ E(K,Y0))  	
*
*									AiΪ��֤���ݷֶΣ�ÿ��128 bits������v bits����λ�� (m-1)*128+v bits��1<= v <=128
*									CiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n-1)*128+u bits��1<= u <=128
*									E(K,Y0)Ϊ��Y0����AES��������
*									MSBu(S)Ϊ����1�ַ������ַ�������S���ݵ������u��bits
* ��ڲ���: 
*						GhashAuthData		Parameter128bits_TypeDef*��������ָ�����洢GHASH(H,A,C)������
*						Counter0				Parameter128bits_TypeDef*��������ָ�����洢Counter0(y0)��ֵ
*						AESKeyMode			ö�ٱ�����ѡ�����ģʽ
*														KEY_Mode_128bits
*														KEY_Mode_192bits
*														KEY_Mode_256bits
*						pKeyAddr  			uint32_t *�ͱ�������ָ�����洢AES key : pKeyAddr[0]Ϊ��λ
*						AuthTag					Parameter128bits_TypeDef*��������ָ�����洢��֤��ǩ����
*
* ���ز���: ��                                      
* 
* ����˵��: ���ܲ���ʱ��pInttextAddr��LenInttextΪ�������룬pOuttextAddrΪ�������
*						���ܲ���ʱ��pInttextAddr��LenInttextΪ�������룬pOuttextAddrΪ�������
*********************************************************************************************************
*/
void GCM_TagAuth(Parameter128bits_TypeDef* GhashAuthData,Parameter128bits_TypeDef* Counter0,AESKeyMode_TypeDef AESKeyMode, uint32_t *pKeyAddr,Parameter128bits_TypeDef* AuthTag)
{
		Parameter128bits_TypeDef	Aes_GCM_temp;
	
		HT_AES_Encrypt(AESKeyMode,pKeyAddr,Counter0->u32);					/*!< ��Counter(Y0)AES���ܲ���	�� E(K,Y0) 					*/
		while(HT_AES_Busy_StatusGet());														/*!< AES������������ж�													*/
		HT_AES_Read(Aes_GCM_temp.u32);														/*!< ��ȡ���������洢��	Aes_GCM_temp						*/	

		HT_AES_Xor128bits(Aes_GCM_temp.u32,GhashAuthData->u32, AuthTag->u32);	
																															/*!< GHASH(H,A,C) ^ E(K,Y0)���㣬�������AuthTag	*/
}

/*
*********************************************************************************************************
*                                GCM ���ܺ���
*
* ����˵��: GCM ���ܺ���
*
* ��ڲ���: GCM_InputStruct    GCM������������ṹ��ָ�룬��Ҫ����8������: 
*                              1) AESKeyMode     		: ���ܽ���ģʽѡ��
*                              2) *pKeyAddr       	: ��Կ��ַָ��
*                              3) *pInitVectorAddr  : ��ʼ������ַָ��
*                              4) LenIV      				: ��ʼ�������ݳ��ȣ�byte��
*                              5) *pPtextAddr  			: �������ݵ�ַָ��
*                              6) LenPtext					: �������ݳ��ȣ�byte��
*                              7) *pAdataAddr  			: ������֤���ݵ�ַָ��
*                              8) LenAdata					: ������֤���ݳ��ȣ�byte��
*						GCM_OutputStruct   GCM������������ṹ��ָ�룬��Ҫ����3������: 
*                              1) *pCtextAddr     	: �Ѽ����������ĵ�ַָ��
*                              2) LenCtext       		: �Ѽ����������ĳ��ȣ�byte��
*                              3) AuthTag  					: ��֤��ǩ���ݵ�ַָ��
*
* ���ز���: ��                                      
* 
* ����˵��:��
*********************************************************************************************************
*/
void GCM_Encrypt(GCM_Encrypt_InputTypedef* GCM_InputStruct, GCM_Encrypt_OutputTypedef* GCM_OutputStruct)
{
		Parameter128bits_TypeDef	Header;																			//Hֵ
		Parameter128bits_TypeDef	Counter0;																		//Counter0ֵ
		Parameter128bits_TypeDef	Ghash_HAC;																	//GHASH(H,A,C)������
	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,ENABLE);									/*��AESģ��ʱ��				*/	

		GCM_GhashHeader_init(GCM_InputStruct->AESKeyMode,GCM_InputStruct->pKeyAddr,Header.u32);				
																																					/*!< ����Header��ֵ			*/
		GCM_Counter0_init(GCM_InputStruct->pInitVectorAddr,GCM_InputStruct->LenIV,Header.u32,&Counter0);		
																																					/*!< ����Counter0��ֵ		*/
		GCM_CTR_Encrypt(GCM_InputStruct->pPtextAddr,GCM_InputStruct->LenPtext,&Counter0,GCM_InputStruct->AESKeyMode,GCM_InputStruct->pKeyAddr,GCM_OutputStruct->pCtextAddr);
																																					/*!< gcm CTR����				*/
		GCM_OutputStruct->LenCtext = GCM_InputStruct->LenPtext;
    GCM_GhashAuth(Header.u32,GCM_InputStruct->pAdataAddr,GCM_InputStruct->LenAdata,GCM_OutputStruct->pCtextAddr,GCM_OutputStruct->LenCtext,&Ghash_HAC);
																																					/*!< GHASH(H.A.C)����			*/
		GCM_TagAuth(&Ghash_HAC,&Counter0,GCM_InputStruct->AESKeyMode,GCM_InputStruct->pKeyAddr,GCM_OutputStruct->AuthTag);			
																																					/*!< ��֤��ǩ����					*/		
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,DISABLE);									/*�ر�AESģ��ʱ��				*/		
}

/*
*********************************************************************************************************
*                                GCM ���ܺ���
*
* ����˵��:  GCM ���ܺ���
*
* ��ڲ���: GCM_InputStruct    GCM������������ṹ��ָ�룬��Ҫ����8������: 
*                              1) AESKeyMode     		: ���ܽ���ģʽѡ��
*                              2) *pKeyAddr       	: ��Կ��ַָ��
*                              3) *pInitVectorAddr  : ��ʼ������ַָ��
*                              4) LenIV      				: ��ʼ�������ݳ��ȣ�byte��
*                              5) *pCtextAddr     	: �������ĵ�ַָ��
*                              6) LenCtext       		: �������ĳ��ȣ�byte�
*                              7) *pAdataAddr  			: ������֤���ݵ�ַָ��
*                              8) LenAdata					: ������֤���ݳ��ȣ�byte��
*                              9) AuthTag  					: ��֤��ǩ���ݵ�ַָ��
*						GCM_OutputStruct   GCM������������ṹ��ָ�룬��Ҫ����3������: 
*                              1) *pPtextAddr  			: �������ݵ�ַָ��
*                              2) LenPtext					: �������ݳ��ȣ�byte��
*
* ���ز���: Bool    = TRUE��  ��֤�ɹ�
*                   = FALSE��	��֤ʧ��                                   
* 
* ����˵��:��
*********************************************************************************************************
*/
Bool GCM_Decrypt(GCM_Decrypt_InputTypedef* GCM_InputStruct, GCM_Decrypt_OutputTypedef* GCM_OutputStruct)
{
		Parameter128bits_TypeDef	Header;																			//Hֵ
		Parameter128bits_TypeDef	Counter0;																		//Counter0ֵ
		Parameter128bits_TypeDef	Ghash_HAC;																	//GHASH(H,A,C)������
		Parameter128bits_TypeDef	AuthTag_Temp;
		uint8_t i;
	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,ENABLE);									/*��AESģ��ʱ��				*/	

		GCM_GhashHeader_init(GCM_InputStruct->AESKeyMode,GCM_InputStruct->pKeyAddr,Header.u32);				
																																					/*!< ����Header��ֵ			*/
		GCM_Counter0_init(GCM_InputStruct->pInitVectorAddr,GCM_InputStruct->LenIV,Header.u32,&Counter0);		
																																					/*!< ����Counter0��ֵ		*/
		GCM_GhashAuth(Header.u32,GCM_InputStruct->pAdataAddr,GCM_InputStruct->LenAdata,GCM_InputStruct->pCtextAddr,GCM_InputStruct->LenCtext,&Ghash_HAC);
																																					/*!< GHASH(H.A.C)����			*/
		GCM_TagAuth(&Ghash_HAC,&Counter0,GCM_InputStruct->AESKeyMode,GCM_InputStruct->pKeyAddr,&AuthTag_Temp);			
																																					/*!< ��֤��ǩ����					*/	
		for(i=0;i<4;i++)
		{
				if(AuthTag_Temp.u32[i] != GCM_InputStruct->AuthTag->u32[i] )
				{
						return	FALSE;																								/*!< ��֤ʧ�ܣ�����	FALSE	*/	
				}
		}
		
		GCM_CTR_Encrypt(GCM_InputStruct->pCtextAddr,GCM_InputStruct->LenCtext,&Counter0,GCM_InputStruct->AESKeyMode,GCM_InputStruct->pKeyAddr,GCM_OutputStruct->pPtextAddr);
																																					/*!< gcm CTR����				*/
		GCM_OutputStruct->LenPtext = GCM_InputStruct->LenCtext;
		
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,DISABLE);								/*�ر�AESģ��ʱ��				*/	
		
		return	TRUE;																													/*!< ���ܳɹ�������	TRUE	*/	
}

/*
*********************************************************************************************************
*                                ECB ���ܼ���
*
* ����˵��: ECB ���ܼ���
*
*						�����㷨��Ci = E(K,Pi)   							,for i=1,2,...,n-1
*											 		 E(K,Pn||0[128-u]))			,for i=n
*									PiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n-1)*128+u bits��1<= u <=128
*									CiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n)*128 bits
*									E(K,Pi)Ϊ��Pi����AES��������
*									Pn||0[128-u]��ΪP���ֶβ���128bits���㣬uΪ���ֶ�λ��
*
* ��ڲ���: ECB_InputStruct    ECB������������ṹ��ָ�룬��Ҫ����4������: 
*                              1) AESKeyMode     		: ���ܽ���ģʽѡ��
*                              2) *pKeyAddr       	: ��Կ��ַָ��
*                              3) *pPtextAddr  			: �������ݵ�ַָ��
*                              4) LenPtext					: �������ݳ��ȣ�byte��
*						ECB_OutputStruct   ECB������������ṹ��ָ�룬��Ҫ����2������: 
*                              1) *pCtextAddr     	: �Ѽ����������ĵ�ַָ��
*                              2) LenCtext       		: �Ѽ����������ĳ��ȣ�byte��
*
* ���ز���: ��                                      
* 
* ����˵��: 
*********************************************************************************************************
*/
void ECB_Encrypt(ECB_Encrypt_InputTypedef* ECB_InputStruct,ECB_Encrypt_OutputTypedef* ECB_OutputStruct)
{
		uint8_t i; 
		Parameter128bits_TypeDef	Aes_ECB_temp;
	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,ENABLE);						/*��AESģ��ʱ��						*/	
	
		if(ECB_InputStruct->LenPtext%16 == 0)														/*!< �������ĳ���						*/
		{
				ECB_OutputStruct->LenCtext = ECB_InputStruct->LenPtext;
		}
		else
		{
				ECB_OutputStruct->LenCtext = ECB_InputStruct->LenPtext - (ECB_InputStruct->LenPtext%16) +16;
		}

	//Ci =  E(K,Pi)   				,for i=1,2,...,n-1
		while(ECB_InputStruct->LenPtext>=16)														/*!< Pλ���ж�	16bytes��128bits��	*/
		{
				for(i=0;i<16;i++)
				{
						Aes_ECB_temp.u8[i] = *ECB_InputStruct->pPtextAddr++;		/*!< Pi������ʱ����Aes_ECB_temp			*/
				}
				HT_AES_Encrypt(ECB_InputStruct->AESKeyMode,ECB_InputStruct->pKeyAddr,Aes_ECB_temp.u32);
																																		/*!< ��Pi AES���ܲ���	�� E(K,Pi) 			*/
				while(HT_AES_Busy_StatusGet());															/*!< AES������������ж�						*/
				HT_AES_Read(Aes_ECB_temp.u32);															/*!< ��ȡ���������洢��	Aes_ECB_temp	*/
				for(i=0;i<16;i++)
				{
						*ECB_OutputStruct->pCtextAddr++ = Aes_ECB_temp.u8[i];	
																																		/*!< �����ı���											*/
				}
				ECB_InputStruct->LenPtext -= 16;														/*!< �¸�128bit�ֶ�									*/			
		}
	//Cn = E(K,Pn||0[128-u])			,for i=n
		if(ECB_InputStruct->LenPtext)
		{
				for(i=0;i<4;i++)
				{
						Aes_ECB_temp.u32[i] = 0x0000;														/*!< Aes_ECB_temp����								*/
				}	
				for(i=16-ECB_InputStruct->LenPtext;i<16;i++)
				{
						Aes_ECB_temp.u8[i] = *ECB_InputStruct->pPtextAddr++;		/*!< Pi������ʱ����Aes_ECB_temp			*/
				}				
				HT_AES_Encrypt(ECB_InputStruct->AESKeyMode,ECB_InputStruct->pKeyAddr,Aes_ECB_temp.u32);
																																		/*!< ��Pi AES���ܲ���	�� E(K,Pi) 		*/
				while(HT_AES_Busy_StatusGet());															/*!< AES������������ж�						*/
				HT_AES_Read(Aes_ECB_temp.u32);															/*!< ��ȡ���������洢��	Aes_ECB_temp		*/
				for(i=0;i<16;i++)
				{
						*ECB_OutputStruct->pCtextAddr++ = Aes_ECB_temp.u8[i];	
				}																														/*!< �����ı���											*/
		}	
		
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,DISABLE);					/*�ر�AESģ��ʱ��										*/	
}	

/*
*********************************************************************************************************
*                                ECB ���ܼ���
*
* ����˵��: ECB ���ܼ���
*
*						�����㷨��Pi = E'(K,Ci)   							,for i=1,2,...,n
*									PiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n)*128 bits
*									CiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n)*128 bits
*									E'(K,Pi)Ϊ��Pi����AES��������
*
* ��ڲ���: ECB_InputStruct    ECB������������ṹ��ָ�룬��Ҫ����4������: 
*                              1) AESKeyMode     		: ���ܽ���ģʽѡ��
*                              2) *pKeyAddr       	: ��Կ��ַָ��
*                              3) *pCtextAddr  			: �������ݵ�ַָ��
*                              4) LenCtext					: �������ݳ��ȣ�byte��
*						ECB_OutputStruct   ECB������������ṹ��ָ�룬��Ҫ����2������: 
*                              1) *pPtextAddr     	: �ѽ������ĵ�ַָ��
*                              2) LenPtext       		: �ѽ������ĳ��ȣ�byte��
*
* ���ز���: Bool    = TRUE��  ���ܳɹ�
*                   = FALSE��	����ʧ�ܣ���������                                      
* 
* ����˵��: 
*********************************************************************************************************
*/
Bool ECB_Decrypt(ECB_Decrypt_InputTypedef* ECB_InputStruct,ECB_Decrypt_OutputTypedef* ECB_OutputStruct)
{
		uint8_t i; 
		Parameter128bits_TypeDef	Aes_ECB_temp;
	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,ENABLE);						/*��AESģ��ʱ��						*/
	
		if(ECB_InputStruct->LenCtext%16 != 0 || ECB_InputStruct->LenCtext == 0)
		{
				return	FALSE;																							/*!< ����λ������										*/
		}
		ECB_OutputStruct->LenPtext = ECB_InputStruct->LenCtext;					/*!< LenPtext = LenCtext						*/
		
	//Pi = E'(K,Ci)   							,for i=1,2,...,n
		while(ECB_InputStruct->LenCtext >= 16)													/*!< Pλ���ж�	16bytes��128bits��	*/
		{
				for(i=0;i<16;i++)
				{
						Aes_ECB_temp.u8[i] = *ECB_InputStruct->pCtextAddr++;		/*!< Pi������ʱ����Aes_ECB_temp			*/
				}
				HT_AES_Decrypt(ECB_InputStruct->AESKeyMode,ECB_InputStruct->pKeyAddr,Aes_ECB_temp.u32);
																																		/*!< ��Pi AES���ܲ���	�� E(K,Pi) 		*/
				while(HT_AES_Busy_StatusGet());															/*!< AES������������ж�						*/
				HT_AES_Read(Aes_ECB_temp.u32);															/*!< ��ȡ���������洢��	Aes_ECB_temp	*/
				for(i=0;i<16;i++)
				{
						*ECB_OutputStruct->pPtextAddr++ = Aes_ECB_temp.u8[i];	
																																		/*!< �����ı���											*/
				}
				ECB_InputStruct->LenCtext -= 16;														/*!< �¸�128bit�ֶ�									*/			
		}
		
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,DISABLE);					/*�ر�AESģ��ʱ��										*/
		
		return TRUE;																										/*!< ���ܳɹ�												*/
}

/*
*********************************************************************************************************
*                                CBC ���ܼ���
*
* ����˵��: CBC ���ܼ���
*
*						�����㷨��Ci = E(K,(P1 ^ IV))									,for i=1
*												 = E(K,(Pi ^	Ci-1))   						,for i=2,3,...,n-1
*											 	 = E(K,((Pn||0[128-u])^Cn-1))			,for i=n
*									PiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n-1)*128+u bits��1<= u <=128
*									CiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n)*128 bits
*									IVΪ��ʼ������0~128 bits
*									E(K,P)Ϊ��P����AES��������
*									Pn||0[128-u]��ΪP���ֶβ���128bits���㣬uΪ���ֶ�λ��
*
* ��ڲ���: CBC_InputStruct    CBC������������ṹ��ָ�룬��Ҫ����4������: 
*                              1) AESKeyMode     		: ���ܽ���ģʽѡ��
*                              2) *pKeyAddr       	: ��Կ��ַָ��
*                              3) *pPtextAddr  			: �������ݵ�ַָ��
*                              4) LenPtext					: �������ݳ��ȣ�byte��
*                              5) *pInitVectorAddr  : ��ʼ������ַָ��
*						CBC_OutputStruct   CBC������������ṹ��ָ�룬��Ҫ����2������: 
*                              1) *pCtextAddr     	: �Ѽ����������ĵ�ַָ��
*                              2) LenCtext       		: �Ѽ����������ĳ��ȣ�byte��
*
* ���ز���: ��                                      
* 
* ����˵��: 
*********************************************************************************************************
*/
void CBC_Encrypt(CBC_Encrypt_InputTypedef* CBC_InputStruct,CBC_Encrypt_OutputTypedef* CBC_OutputStruct)
{
		uint8_t i; 
		Parameter128bits_TypeDef	Aes_CBC_temp;
	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,ENABLE);						/*��AESģ��ʱ��						*/
	
		if(CBC_InputStruct->LenPtext%16 == 0)														/*!< �������ĳ���						*/
		{
				CBC_OutputStruct->LenCtext = CBC_InputStruct->LenPtext;
		}
		else
		{
				CBC_OutputStruct->LenCtext = CBC_InputStruct->LenPtext - (CBC_InputStruct->LenPtext%16) +16;
		}
		
/* LenPtext>=16	*/
		if(CBC_InputStruct->LenPtext>=16)																/*!< Pλ���ж�	16bytes��128bits��	*/
		{
	//Ci = E(K,(P1 ^ IV))									,for i=1		
				for(i=0;i<16;i++)
				{
						Aes_CBC_temp.u8[i] = *CBC_InputStruct->pPtextAddr++ ^ CBC_InputStruct->pInitVectorAddr->u8[i];		
																																		/*!< (P1 ^ IV)���㣬�������Aes_CBC_temp			*/
				}
				HT_AES_Encrypt(CBC_InputStruct->AESKeyMode,CBC_InputStruct->pKeyAddr,Aes_CBC_temp.u32);
																																		/*!< ��(P1 ^ IV) AES���ܲ�����E(K,(P1 ^ IV))	*/
				while(HT_AES_Busy_StatusGet());															/*!< AES������������ж�						*/
				HT_AES_Read(Aes_CBC_temp.u32);															/*!< ��ȡ���������洢��	Aes_CBC_temp	*/
				for(i=0;i<16;i++)
				{
						*CBC_OutputStruct->pCtextAddr++ = Aes_CBC_temp.u8[i];	
																																		/*!< ������C1����										*/
				}
				CBC_InputStruct->LenPtext -= 16;														/*!< �¸�128bit�ֶ�									*/
				CBC_OutputStruct->pCtextAddr -= 16;													/*!< ָ��ָ��C1											*/	

	//Ci =  E(K,(Pi ^	Ci-1))  				,for i=2,3,...,n-1
				while(CBC_InputStruct->LenPtext>=16)												/*!< Pλ���ж�	16bytes��128bits��	*/
				{
						for(i=0;i<16;i++)
						{
								Aes_CBC_temp.u8[i] = *CBC_InputStruct->pPtextAddr++ ^ *CBC_OutputStruct->pCtextAddr++;		
																																		/*!< (Pi ^ Ci-1)���㣬�������Aes_CBC_temp			*/
						}
						HT_AES_Encrypt(CBC_InputStruct->AESKeyMode,CBC_InputStruct->pKeyAddr,Aes_CBC_temp.u32);
																																		/*!< ��(Pi ^ Ci-1) AES���ܲ�����E(K,(Pi ^ Ci-1))	*/
						while(HT_AES_Busy_StatusGet());													/*!< AES������������ж�						*/
						HT_AES_Read(Aes_CBC_temp.u32);													/*!< ��ȡ���������洢��	Aes_CBC_temp	*/
						for(i=0;i<16;i++)
						{
								*CBC_OutputStruct->pCtextAddr++ = Aes_CBC_temp.u8[i];	
																																		/*!< �����ı���											*/
						}
						CBC_InputStruct->LenPtext -= 16;												/*!< �¸�128bit�ֶ�									*/	
						CBC_OutputStruct->pCtextAddr-=16;												/*!< ָ��ָ��Ci-1										*/
				}
	//E(K,((Pn||0[128-u])^Cn-1))			,for i=n
				if(CBC_InputStruct->LenPtext)
				{
						for(i=0;i<4;i++)
						{
								Aes_CBC_temp.u32[i] = 0x0000;														/*!< Aes_CBC_temp����								*/
						}	
						for(i=16-CBC_InputStruct->LenPtext;i<16;i++)
						{
								Aes_CBC_temp.u8[i] = *CBC_InputStruct->pPtextAddr++;		/*!< Pi������ʱ����Aes_CBC_temp������	*/
						}		
						for(i=0;i<16;i++)
						{
								Aes_CBC_temp.u8[i] = Aes_CBC_temp.u8[i] ^ *CBC_OutputStruct->pCtextAddr++;		
																																				/*!< ((Pn||0[128-u])^Cn-1)���㣬�������Aes_CBC_temp			*/
						}
						HT_AES_Encrypt(CBC_InputStruct->AESKeyMode,CBC_InputStruct->pKeyAddr,Aes_CBC_temp.u32);
																																				/*!< ��((Pn||0[128-u])^Ci-1) AES���ܲ���	�� E(K,Pi) 		*/
						while(HT_AES_Busy_StatusGet());															/*!< AES������������ж�						*/
						HT_AES_Read(Aes_CBC_temp.u32);															/*!< ��ȡ���������洢��	Aes_CBC_temp		*/
						for(i=0;i<16;i++)
						{
								*CBC_OutputStruct->pCtextAddr++ = Aes_CBC_temp.u8[i];	
						}																														/*!< �����ı���											*/
				}	
		}
/* LenPtext<16 ��!=0	*/
		else if(CBC_InputStruct->LenPtext)
		{
	//Ci = E(K,((P1||0[128-U]) ^ IV))									,for i=n=1	
				for(i=0;i<4;i++)
				{
						Aes_CBC_temp.u32[i] = 0x0000;														/*!< Aes_CBC_temp����								*/
				}	
				for(i=16-CBC_InputStruct->LenPtext;i<16;i++)
				{
						Aes_CBC_temp.u8[i] = *CBC_InputStruct->pPtextAddr++;		/*!< Pi������ʱ����Aes_CBC_temp,����*/
				}		
				for(i=0;i<16;i++)
				{
						Aes_CBC_temp.u8[i] = Aes_CBC_temp.u8[i] ^ CBC_InputStruct->pInitVectorAddr->u8[i];		
																																		/*!< (P1 ^ IV)���㣬�������Aes_CBC_temp			*/
				}
				HT_AES_Encrypt(CBC_InputStruct->AESKeyMode,CBC_InputStruct->pKeyAddr,Aes_CBC_temp.u32);
																																		/*!< ��(P1 ^ IV)AES���ܲ���	�� E(K,(P1 ^ IV)) 		*/
				while(HT_AES_Busy_StatusGet());															/*!< AES������������ж�						*/
				HT_AES_Read(Aes_CBC_temp.u32);															/*!< ��ȡ���������洢��	Aes_CBC_temp		*/
				for(i=0;i<16;i++)
				{
						*CBC_OutputStruct->pCtextAddr++ = Aes_CBC_temp.u8[i];	
				}																														/*!< �����ı���											*/
		}
		
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,DISABLE);					/*�ر�AESģ��ʱ��										*/
}	

/*
*********************************************************************************************************
*                                CBC ���ܼ���
*
* ����˵��: CBC ���ܼ���
*
*						�����㷨��Pi = E'(K,C1) ^ IV									,for i=1
*												 = E'(K,Ci) ^	Ci-1								,for i=2,3,...,n
*									PiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n)*128 bits
*									CiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n)*128 bits
*									IVΪ��ʼ������0~128 bits
*									E'(K,C)Ϊ��C����AES��������
*
* ��ڲ���: CBC_InputStruct    CBC������������ṹ��ָ�룬��Ҫ����4������: 
*                              1) AESKeyMode     		: ���ܽ���ģʽѡ��
*                              2) *pKeyAddr       	: ��Կ��ַָ��
*                              3) *pCtextAddr  			: �������ݵ�ַָ��
*                              4) LenCtext					: �������ݳ��ȣ�byte��
*                              5) *pInitVectorAddr  : ��ʼ������ַָ��
*						CBC_OutputStruct   CBC������������ṹ��ָ�룬��Ҫ����2������: 
*                              1) *pPtextAddr     	: �ѽ������ĵ�ַָ��
*                              2) LenPtext       		: �ѽ������ĳ��ȣ�byte��
*
* ���ز���: Bool    = TRUE��  ���ܳɹ�
*                   = FALSE��	����ʧ�ܣ���������   
* ����˵��: 
*********************************************************************************************************
*/
Bool CBC_Decrypt(CBC_Decrypt_InputTypedef* CBC_InputStruct,CBC_Decrypt_OutputTypedef* CBC_OutputStruct)
{
		uint8_t i; 
		Parameter128bits_TypeDef	Aes_CBC_temp;
	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,ENABLE);						/*��AESģ��ʱ��						*/
	
		if(CBC_InputStruct->LenCtext%16 != 0 || CBC_InputStruct->LenCtext == 0)
		{
				return	FALSE;																							/*!< ����λ������										*/
		}
		CBC_OutputStruct->LenPtext = CBC_InputStruct->LenCtext;					/*!< LenPtext = LenCtext						*/
		
	//Pi = E'(K,C1) ^ IV									,for i=1
		for(i=0;i<16;i++)
		{
				Aes_CBC_temp.u8[i] = *CBC_InputStruct->pCtextAddr++;				/*!< C1������ʱ����Aes_CBC_temp			*/
		}
		HT_AES_Decrypt(CBC_InputStruct->AESKeyMode,CBC_InputStruct->pKeyAddr,Aes_CBC_temp.u32);
																																		/*!< ��C1 AES���ܲ�����E'(K,C1)			*/
		while(HT_AES_Busy_StatusGet());																	/*!< AES������������ж�						*/
		HT_AES_Read(Aes_CBC_temp.u32);																	/*!< ��ȡ���������洢��	Aes_CBC_temp	*/
		for(i=0;i<16;i++)
		{
				*CBC_OutputStruct->pPtextAddr++ = Aes_CBC_temp.u8[i] ^ CBC_InputStruct->pInitVectorAddr->u8[i];	
																																		/*!< ������P1����										*/
		}
		CBC_InputStruct->LenCtext -= 16;																/*!< �¸�128bit�ֶ�									*/

	//Pi = E'(K,Ci) ^	Ci-1								,for i=2,3,...,n
		while(CBC_InputStruct->LenCtext>=16)														/*!< Pλ���ж�	16bytes��128bits��	*/
		{
				for(i=0;i<16;i++)
				{
						Aes_CBC_temp.u8[i] = *CBC_InputStruct->pCtextAddr++;		/*!< Ci������ʱ����Aes_CBC_temp			*/
				}
				CBC_InputStruct->pCtextAddr -= 32;														/*!< ָ��ָ��Ci-1										*/
				HT_AES_Decrypt(CBC_InputStruct->AESKeyMode,CBC_InputStruct->pKeyAddr,Aes_CBC_temp.u32);
																																		/*!< ��Ci AES���ܲ�����E'(K,Ci)			*/
				while(HT_AES_Busy_StatusGet());															/*!< AES������������ж�						*/
				HT_AES_Read(Aes_CBC_temp.u32);															/*!< ��ȡ���������洢��	Aes_CBC_temp	*/
				for(i=0;i<16;i++)
				{
						*CBC_OutputStruct->pPtextAddr++ = Aes_CBC_temp.u8[i] ^ *CBC_InputStruct->pCtextAddr++;	
																																		/*!< ������P1����,Pi = E'(K,Ci) ^	Ci-1	*/
				}
				CBC_InputStruct->pCtextAddr += 16;													/*!< ָ��ָ��Ci+1										*/
				CBC_InputStruct->LenCtext -= 16;														/*!< �¸�128bit�ֶ�									*/
		}
		
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,DISABLE);					/*�ر�AESģ��ʱ��										*/
		
		return TRUE;																										/*!< ���ܳɹ�												*/
}	

/*
*********************************************************************************************************
*                                CTR����/���ܼ���
*
* ����˵��: CTR����/���ܼ���
*						�����㷨��Ci = Pi ^ E(K,Yi)   				,for i=1,2,...,n-1
*											 		 Pn ^ MSBu(E(K,Yn))			,for i=n
*						�����㷨��Pi = Ci ^ E(K,Yi)   				,for i=1,2,...,n-1
*											 		 Cn ^ MSBu(E(K,Yn))			,for i=n
*									PiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n-1)*128+u bits��1<= u <=128
*									CiΪ���ķֶΣ�ÿ��128 bits������u bits����λ�� (n-1)*128+u bits��1<= u <=128
*									E(K,Yi)Ϊ��Yi����AES��������
*									MSBu(S)Ϊ����1�ַ������ַ�������S���ݵ������u��bits
*
* ��ڲ���: CTR_InputStruct    CTR������������ṹ��ָ�룬��Ҫ����5������: 
*                              1) AESKeyMode     		: ���ܽ���ģʽѡ��
*                              2) *pKeyAddr       	: ��Կ��ַָ��
*                              3) *pInttextAddr  		: ���ӽ������ݵ�ַָ��
*                              4) LenInttext				: ���ӽ������ݳ��ȣ�byte��
*                              5) *Counter0  				: Counter0��ַָ��
*						CTR_OutputStruct   CTR������������ṹ��ָ�룬��Ҫ����2������: 
*                              1) *pOuttextAddr     : �Ѽ�/�������ݵ�ַָ��
*                              2) LenOuttext       	: �Ѽ�/�������ݳ��ȣ�byte��
*
* ���ز���: ��                                      
* 
* ����˵��: ���ܲ���ʱ��pInttextAddr��LenInttextΪ�������룬pOuttextAddrΪ�������
*						���ܲ���ʱ��pInttextAddr��LenInttextΪ�������룬pOuttextAddrΪ�������
*********************************************************************************************************
*/
void CTR_Encrypt(CTR_Encrypt_InputTypedef* CTR_InputStruct,CTR_Encrypt_OutputTypedef* CTR_OutputStruct)
{
		uint8_t i; 
		Parameter128bits_TypeDef	Counter_temp,Aes_GCM_temp;
	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,ENABLE);						/*��AESģ��ʱ��						*/
	
		CTR_OutputStruct->LenOuttext = CTR_InputStruct->LenInttext;
																															/*!< LenOuttext = LenInttext				*/
		for(i=0;i<4;i++)
		{
				Counter_temp.u32[i] = CTR_InputStruct->Counter0->u32[i]; 
																															/*!< counter0�浽Counter_temp����		*/
		}
	//Ci = Pi ^ E(K,Yi)   				,for i=1,2,...,n-1
		while(CTR_InputStruct->LenInttext>=16)										/*!< Pλ���ж�	16bytes��128bits��	*/
		{
				GCM_Counter_increase(&Counter_temp);									/*!< GCM Counter(Yi)�ۼӼ���				*/
				HT_AES_Encrypt(CTR_InputStruct->AESKeyMode,CTR_InputStruct->pKeyAddr,Counter_temp.u32);
																															/*!< ��Counter(Yi)AES���ܲ���	�� E(K,Yi) 			*/
				while(HT_AES_Busy_StatusGet());												/*!< AES������������ж�						*/
				HT_AES_Read(Aes_GCM_temp.u32);												/*!< ��ȡ���������洢��	Aes_GCM_temp*/
				for(i=0;i<16;i++)
				{
						*CTR_OutputStruct->pOuttextAddr++ = Aes_GCM_temp.u8[i] ^ *CTR_InputStruct->pInttextAddr++;	
																															/*!< Pi ^ E(K,Yi)����								*/
				}
				CTR_InputStruct->LenInttext -= 16;										/*!< �¸�128bit�ֶ�									*/			
		}
	//Cn = Pn ^ MSBu(E(K,Yn))			,for i=n
		if(CTR_InputStruct->LenInttext)
		{
				GCM_Counter_increase(&Counter_temp);									/*!< GCM Counter(Yi)�ۼӼ���				*/
				HT_AES_Encrypt(CTR_InputStruct->AESKeyMode,CTR_InputStruct->pKeyAddr,Counter_temp.u32);
																															/*!< ��Counter(Yn)AES���ܲ���	�� E(K,Yn) 	*/
				while(HT_AES_Busy_StatusGet());												/*!< AES������������ж�						*/
				HT_AES_Read(Aes_GCM_temp.u32);												/*!< ��ȡ���������洢��	Aes_GCM_temp1		*/
				for(i=16-CTR_InputStruct->LenInttext;i<16;i++)
				{
						*CTR_OutputStruct->pOuttextAddr++ = Aes_GCM_temp.u8[i] ^ *CTR_InputStruct->pInttextAddr++;							
																															/*!< Pn ^ MSBu(E(K,Yn))����					*/
				}
		}	
		HT_CMU_ClkCtrl0Config(CMU_CLKCTRL0_ADESRADEN,DISABLE);		/*�ر�AESģ��ʱ��						*/
}

/*
*********************************************************************************************************
*                                GCM_AES EXAMPLES FOR HT6x2x/HT501x
*
* ����˵��: ���ÿ⺯��ʵ��GCM_AES��/����
*
* ��ڲ���: ��                            
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
/*��������*/		
		Parameter256bits_TypeDef	AesKey;							//��Կ
		AESKeyMode_TypeDef	AESKeyMode;								//���ܷ�ʽ
		Parameter512bits_TypeDef	Plaintext;					//����
		uint64_t	LenPtext;														//����λ��
		Parameter512bits_TypeDef	Ciphertext;					//����
		uint64_t	LenCtext;														//����λ��
		Parameter256bits_TypeDef	AddAuthData;				//������֤����
		uint64_t	LenAdata;														//������֤����λ��
		Parameter512bits_TypeDef	InitVector;					//��ʼ����
		uint64_t	LenIV;															//��ʼ����λ��
		Parameter128bits_TypeDef	AuthTag;						//��֤��ǩ
		Bool DecryptSta;															//���ܳɹ�״̬
		
		Parameter128bits_TypeDef	CTR_Counter0;				//CTR��ʼ������ֵ
		
void AES_GCM_Example(uint8_t TestCase)
{   
		CMU_InitTypeDef  CMU_InitStructure;
		GCM_Encrypt_InputTypedef GCM_Encrypt_InputStruct;	
		GCM_Encrypt_OutputTypedef	GCM_Encrypt_OutputStruct;
		GCM_Decrypt_InputTypedef GCM_Decrypt_InputStruct;	
		GCM_Decrypt_OutputTypedef	GCM_Decrypt_OutputStruct;
		uint8_t i;
	
/*���������ʼ��*/
		GCM_TestCase_Init(TestCase);
/*����ϵͳʱ��*/    
    CMU_InitStructure.SysClkSel = SysPLL;
    CMU_InitStructure.CPUDiv = CPUDiv1;
    HT_CMU_Init(&CMU_InitStructure);
	

/*��������ṹ���ʼ��*/		
		for(i=0;i<16;i++)
		{
				Ciphertext.u32[i] = 0x0000;
				AuthTag.u8[i] = 0x00;
		}
		GCM_Encrypt_InputStruct.AESKeyMode = AESKeyMode;
		GCM_Encrypt_InputStruct.pKeyAddr = AesKey.u32;
		GCM_Encrypt_InputStruct.pInitVectorAddr = InitVector.u8;
		GCM_Encrypt_InputStruct.LenIV = LenIV;
		GCM_Encrypt_InputStruct.pPtextAddr = Plaintext.u8;
		GCM_Encrypt_InputStruct.LenPtext = LenPtext;
		GCM_Encrypt_InputStruct.pAdataAddr = AddAuthData.u8;
		GCM_Encrypt_InputStruct.LenAdata = LenAdata;
		GCM_Encrypt_OutputStruct.AuthTag = &AuthTag;
		GCM_Encrypt_OutputStruct.pCtextAddr = Ciphertext.u8;
		GCM_Encrypt(&GCM_Encrypt_InputStruct,&GCM_Encrypt_OutputStruct);										/*!< gcm ����	*/
		
/*��������ṹ���ʼ��*/	
		for(i=0;i<16;i++)
		{
				Plaintext.u32[i] = 0x0000;
		}
		GCM_Decrypt_InputStruct.AESKeyMode = AESKeyMode;
		GCM_Decrypt_InputStruct.pKeyAddr = AesKey.u32;
		GCM_Decrypt_InputStruct.pInitVectorAddr = InitVector.u8;
		GCM_Decrypt_InputStruct.LenIV = LenIV;
		GCM_Decrypt_InputStruct.pCtextAddr = GCM_Encrypt_OutputStruct.pCtextAddr;
		GCM_Decrypt_InputStruct.LenCtext = GCM_Encrypt_OutputStruct.LenCtext;
		GCM_Decrypt_InputStruct.pAdataAddr = AddAuthData.u8;
		GCM_Decrypt_InputStruct.LenAdata = LenAdata;
		GCM_Decrypt_InputStruct.AuthTag = &AuthTag;
		GCM_Decrypt_OutputStruct.pPtextAddr = Plaintext.u8;
		DecryptSta = GCM_Decrypt(&GCM_Decrypt_InputStruct, &GCM_Decrypt_OutputStruct);			/*!< gcm ����	*/
} 

/*
*********************************************************************************************************
*                                CTR_AES EXAMPLES FOR HT6x2x/HT501x
*
* ����˵��: ���ÿ⺯��ʵ��CTR_AES��/����
*
* ��ڲ���: ��                            
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void AES_CTR_Example(void)
{   
		CMU_InitTypeDef  CMU_InitStructure;
		CTR_Encrypt_InputTypedef CTR_Encrypt_InputStruct;	
		CTR_Encrypt_OutputTypedef	CTR_Encrypt_OutputStruct;
		uint8_t i;
	
/*���������ʼ��*/
		CTR_TestCase_Init();
/*����ϵͳʱ��*/    
    CMU_InitStructure.SysClkSel = SysPLL;
    CMU_InitStructure.CPUDiv = CPUDiv1;
    HT_CMU_Init(&CMU_InitStructure);

/*��������ṹ���ʼ��*/		
		for(i=0;i<16;i++)
		{
				Ciphertext.u32[i] = 0x0000;
		}
		CTR_Encrypt_InputStruct.AESKeyMode = AESKeyMode;
		CTR_Encrypt_InputStruct.pKeyAddr = AesKey.u32;
		CTR_Encrypt_InputStruct.pInttextAddr = Plaintext.u8;
		CTR_Encrypt_InputStruct.LenInttext= LenPtext;
		CTR_Encrypt_InputStruct.Counter0 = &CTR_Counter0;
		CTR_Encrypt_OutputStruct.pOuttextAddr = Ciphertext.u8;
		CTR_Encrypt(&CTR_Encrypt_InputStruct,&CTR_Encrypt_OutputStruct);										/*!< CTR ����	*/
		
/*��������ṹ���ʼ��*/
		LenCtext = CTR_Encrypt_OutputStruct.LenOuttext;
		for(i=0;i<16;i++)
		{
				Plaintext.u32[i] = 0x0000;
		}
		CTR_Encrypt_InputStruct.AESKeyMode = AESKeyMode;
		CTR_Encrypt_InputStruct.pKeyAddr = AesKey.u32;
		CTR_Encrypt_InputStruct.pInttextAddr = Ciphertext.u8;
		CTR_Encrypt_InputStruct.LenInttext= LenCtext;
		CTR_Encrypt_InputStruct.Counter0 = &CTR_Counter0;
		CTR_Encrypt_OutputStruct.pOuttextAddr = Plaintext.u8;
		CTR_Encrypt(&CTR_Encrypt_InputStruct,&CTR_Encrypt_OutputStruct);										/*!< CTR ����	*/
} 
/*
*********************************************************************************************************
*                                CBC_AES EXAMPLES FOR HT6x2x/HT501x
*
* ����˵��: ���ÿ⺯��ʵ��CBC_AES��/����
*
* ��ڲ���: ��                            
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void AES_CBC_Example(void)
{   
		CMU_InitTypeDef  CMU_InitStructure;
		CBC_Encrypt_InputTypedef CBC_Encrypt_InputStruct;	
		CBC_Encrypt_OutputTypedef	CBC_Encrypt_OutputStruct;
		CBC_Decrypt_InputTypedef CBC_Decrypt_InputStruct;	
		CBC_Decrypt_OutputTypedef	CBC_Decrypt_OutputStruct;
		uint8_t i;
	
/*���������ʼ��*/
		CBC_TestCase_Init();
/*����ϵͳʱ��*/    
    CMU_InitStructure.SysClkSel = SysPLL;
    CMU_InitStructure.CPUDiv = CPUDiv1;
    HT_CMU_Init(&CMU_InitStructure);

/*��������ṹ���ʼ��*/		
		for(i=0;i<16;i++)
		{
				Ciphertext.u32[i] = 0x0000;
		}
		CBC_Encrypt_InputStruct.AESKeyMode = AESKeyMode;
		CBC_Encrypt_InputStruct.pKeyAddr = AesKey.u32;
		CBC_Encrypt_InputStruct.pPtextAddr = Plaintext.u8;
		CBC_Encrypt_InputStruct.LenPtext= LenPtext;
		CBC_Encrypt_InputStruct.pInitVectorAddr = &CTR_Counter0;
		CBC_Encrypt_OutputStruct.pCtextAddr = Ciphertext.u8;
		CBC_Encrypt(&CBC_Encrypt_InputStruct,&CBC_Encrypt_OutputStruct);										/*!< CBC ����	*/
		
/*��������ṹ���ʼ��*/
		LenCtext = CBC_Encrypt_OutputStruct.LenCtext;
		for(i=0;i<16;i++)
		{
				Plaintext.u32[i] = 0x0000;
		}
		CBC_Decrypt_InputStruct.AESKeyMode = AESKeyMode;
		CBC_Decrypt_InputStruct.pKeyAddr = AesKey.u32;
		CBC_Decrypt_InputStruct.pCtextAddr = Ciphertext.u8;
		CBC_Decrypt_InputStruct.LenCtext= LenCtext;
		CBC_Decrypt_InputStruct.pInitVectorAddr= &CTR_Counter0;
		CBC_Decrypt_OutputStruct.pPtextAddr = Plaintext.u8;
		DecryptSta = CBC_Decrypt(&CBC_Decrypt_InputStruct,&CBC_Decrypt_OutputStruct);										/*!< CBC ����	*/
} 

/*
*********************************************************************************************************
*                                ECB_AES EXAMPLES FOR HT6x2x/HT501x
*
* ����˵��: ���ÿ⺯��ʵ��ECB_AES��/����
*
* ��ڲ���: ��                            
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void AES_ECB_Example(void)
{   
		CMU_InitTypeDef  CMU_InitStructure;
		ECB_Encrypt_InputTypedef ECB_Encrypt_InputStruct;	
		ECB_Encrypt_OutputTypedef	ECB_Encrypt_OutputStruct;
		ECB_Decrypt_InputTypedef ECB_Decrypt_InputStruct;	
		ECB_Decrypt_OutputTypedef	ECB_Decrypt_OutputStruct;
		uint8_t i;
	
/*���������ʼ��*/
		ECB_TestCase_Init();
/*����ϵͳʱ��*/    
    CMU_InitStructure.SysClkSel = SysPLL;
    CMU_InitStructure.CPUDiv = CPUDiv1;
    HT_CMU_Init(&CMU_InitStructure);

/*��������ṹ���ʼ��*/		
		for(i=0;i<16;i++)
		{
				Ciphertext.u32[i] = 0x0000;
		}
		ECB_Encrypt_InputStruct.AESKeyMode = AESKeyMode;
		ECB_Encrypt_InputStruct.pKeyAddr = AesKey.u32;
		ECB_Encrypt_InputStruct.pPtextAddr = Plaintext.u8;
		ECB_Encrypt_InputStruct.LenPtext= LenPtext;
		ECB_Encrypt_OutputStruct.pCtextAddr = Ciphertext.u8;
		ECB_Encrypt(&ECB_Encrypt_InputStruct,&ECB_Encrypt_OutputStruct);										/*!< ECB ����	*/
		
/*��������ṹ���ʼ��*/
		LenCtext = ECB_Encrypt_OutputStruct.LenCtext;
		for(i=0;i<16;i++)
		{
				Plaintext.u32[i] = 0x0000;
		}
		ECB_Decrypt_InputStruct.AESKeyMode = AESKeyMode;
		ECB_Decrypt_InputStruct.pKeyAddr = AesKey.u32;
		ECB_Decrypt_InputStruct.pCtextAddr = Ciphertext.u8;
		ECB_Decrypt_InputStruct.LenCtext= LenCtext;
		ECB_Decrypt_OutputStruct.pPtextAddr = Plaintext.u8;
		DecryptSta = ECB_Decrypt(&ECB_Decrypt_InputStruct,&ECB_Decrypt_OutputStruct);										/*!< ECB ����	*/
} 

/*
*********************************************************************************************************
*                                TEST CASE INITIAL
*
* ����˵��: �������㷨��֤testcase��ʼ��
*
* ��ڲ���: ��                            
*
* ���ز���: ��                                      
* 
* ����˵��: �˺���Ĭ�ϲ����빤�̱���
*********************************************************************************************************
*/
void GCM_TestCase_Init(uint8_t TestCase)
{
		switch(TestCase)
		{		
		case 1:
				AESKeyMode = KEY_Mode_128bits;
				AesKey.u32[0]		= 	0x00000000;
				AesKey.u32[1]		= 	0x00000000;
				AesKey.u32[2]		= 	0x00000000;
				AesKey.u32[3]		= 	0x00000000;
				
				LenPtext	=  0;
				
				LenIV = 12;
				InitVector.u32[0] = 0x00000000;
				InitVector.u32[1] = 0x00000000;
				InitVector.u32[2] = 0x00000000;
				
				LenAdata = 0;
				
				break;
		
		case 2:
				AESKeyMode = KEY_Mode_128bits;
				AesKey.u32[0]		= 	0x00000000;
				AesKey.u32[1]		= 	0x00000000;
				AesKey.u32[2]		= 	0x00000000;
				AesKey.u32[3]		= 	0x00000000;
				
				LenPtext	=  16;
				Plaintext.u32[0]		= 	0x00000000;
				Plaintext.u32[1]		= 	0x00000000;
				Plaintext.u32[2]		= 	0x00000000;
				Plaintext.u32[3]		= 	0x00000000;
				
				LenIV = 12;
				InitVector.u32[0] = 0x00000000;
				InitVector.u32[1] = 0x00000000;
				InitVector.u32[2] = 0x00000000;
						
				LenAdata = 0;	
				break;

		case 3:
				AESKeyMode = KEY_Mode_128bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;
				
				LenPtext	=  16*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[13] 	= 	0xba637b39;
				Plaintext.u32[14] 	= 	0xaa0de657;
				Plaintext.u32[15] 	= 	0xb16aedf5;
				
				LenIV = 12;
				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[1] = 0xfacedbad;
				InitVector.u32[2] = 0xcafebabe;
						
				LenAdata = 0;	
				break;
				
		case 4:
				AESKeyMode = KEY_Mode_128bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;
				
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				
				LenIV = 3*4;
				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[1] = 0xfacedbad;
				InitVector.u32[2] = 0xcafebabe;
				break;
				
		case 5:
				AESKeyMode = KEY_Mode_128bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;
				
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 2*4;
//				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[0] = 0xfacedbad;
				InitVector.u32[1] = 0xcafebabe;
				break;

		case 6:
				AESKeyMode = KEY_Mode_128bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;
				
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 15*4;
				InitVector.u32[0] = 0xff5269aa;
				InitVector.u32[1] = 0x55909c5a;
				InitVector.u32[2] = 0xf88406e5;
				InitVector.u32[3] = 0x9313225d;
				InitVector.u32[4] = 0xa318a728;
				InitVector.u32[5] = 0xe4c303d2;
				InitVector.u32[6] = 0x534f7da1;
				InitVector.u32[7] = 0x6a7a9538;
				InitVector.u32[8] = 0x9a6b5254;
				InitVector.u32[9] = 0xfcf0e242;
				InitVector.u32[10] = 0x56809539;
				InitVector.u32[11] = 0xc3c0c951;
				InitVector.u32[12] = 0xa637b39b;
				InitVector.u32[13] = 0xa0de6a57;
				InitVector.u32[14] = 0x16aedbf5;
				break;

		case 7:
				AESKeyMode = KEY_Mode_192bits;
				AesKey.u32[0]		= 	0x00000000;
				AesKey.u32[1]		= 	0x00000000;
				AesKey.u32[2]		= 	0x00000000;
				AesKey.u32[3]		= 	0x00000000;
				AesKey.u32[4]		= 	0x00000000;
				AesKey.u32[5]		= 	0x00000000;		
				
				LenPtext	=  0;
				
				LenIV = 12;
				InitVector.u32[0] = 0x00000000;
				InitVector.u32[1] = 0x00000000;
				InitVector.u32[2] = 0x00000000;
				
				LenAdata = 0;
				break;

		case 8:
				AESKeyMode = KEY_Mode_192bits;
				AesKey.u32[0]		= 	0x00000000;
				AesKey.u32[1]		= 	0x00000000;
				AesKey.u32[2]		= 	0x00000000;
				AesKey.u32[3]		= 	0x00000000;
				AesKey.u32[4]		= 	0x00000000;
				AesKey.u32[5]		= 	0x00000000;	
				
				LenPtext	=  16;
				Plaintext.u32[0]		= 	0x00000000;
				Plaintext.u32[1]		= 	0x00000000;
				Plaintext.u32[2]		= 	0x00000000;
				Plaintext.u32[3]		= 	0x00000000;
				
				LenIV = 12;
				InitVector.u32[0] = 0x00000000;
				InitVector.u32[1] = 0x00000000;
				InitVector.u32[2] = 0x00000000;
						
				LenAdata = 0;	
				break;	

	case 9:
				AESKeyMode = KEY_Mode_192bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
	
				LenPtext	=  16*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[13] 	= 	0xba637b39;
				Plaintext.u32[14] 	= 	0xaa0de657;
				Plaintext.u32[15] 	= 	0xb16aedf5;	
				
				LenAdata = 0;	
				
				LenIV = 3*4;
				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[1] = 0xfacedbad;
				InitVector.u32[2] = 0xcafebabe;				
				break;
	case 10:
				AESKeyMode = KEY_Mode_192bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 3*4;
				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[1] = 0xfacedbad;
				InitVector.u32[2] = 0xcafebabe;	
				break;
				
	case 11:
				AESKeyMode = KEY_Mode_192bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 2*4;
//				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[0] = 0xfacedbad;
				InitVector.u32[1] = 0xcafebabe;	
				break;
				
	case 12:
				AESKeyMode = KEY_Mode_192bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 15*4;
				InitVector.u32[0] = 0xff5269aa;
				InitVector.u32[1] = 0x55909c5a;
				InitVector.u32[2] = 0xf88406e5;
				InitVector.u32[3] = 0x9313225d;
				InitVector.u32[4] = 0xa318a728;
				InitVector.u32[5] = 0xe4c303d2;
				InitVector.u32[6] = 0x534f7da1;
				InitVector.u32[7] = 0x6a7a9538;
				InitVector.u32[8] = 0x9a6b5254;
				InitVector.u32[9] = 0xfcf0e242;
				InitVector.u32[10] = 0x56809539;
				InitVector.u32[11] = 0xc3c0c951;
				InitVector.u32[12] = 0xa637b39b;
				InitVector.u32[13] = 0xa0de6a57;
				InitVector.u32[14] = 0x16aedbf5;
				break;		

		case 13:
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0x00000000;
				AesKey.u32[1]		= 	0x00000000;
				AesKey.u32[2]		= 	0x00000000;
				AesKey.u32[3]		= 	0x00000000;
				AesKey.u32[4]		= 	0x00000000;
				AesKey.u32[5]		= 	0x00000000;		
				AesKey.u32[6]		= 	0x00000000;
				AesKey.u32[7]		= 	0x00000000;	
				
				LenPtext	=  0;
				
				LenIV = 12;
				InitVector.u32[0] = 0x00000000;
				InitVector.u32[1] = 0x00000000;
				InitVector.u32[2] = 0x00000000;
				
				LenAdata = 0;
				break;

		case 14:
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0x00000000;
				AesKey.u32[1]		= 	0x00000000;
				AesKey.u32[2]		= 	0x00000000;
				AesKey.u32[3]		= 	0x00000000;
				AesKey.u32[4]		= 	0x00000000;
				AesKey.u32[5]		= 	0x00000000;		
				AesKey.u32[6]		= 	0x00000000;
				AesKey.u32[7]		= 	0x00000000;	
				
				LenPtext	=  16;
				Plaintext.u32[0]		= 	0x00000000;
				Plaintext.u32[1]		= 	0x00000000;
				Plaintext.u32[2]		= 	0x00000000;
				Plaintext.u32[3]		= 	0x00000000;
				
				LenIV = 12;
				InitVector.u32[0] = 0x00000000;
				InitVector.u32[1] = 0x00000000;
				InitVector.u32[2] = 0x00000000;
						
				LenAdata = 0;	
				break;	

	case 15:
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
				AesKey.u32[6]		= 	0x6d6a8f94;
				AesKey.u32[7]		= 	0x67308308;	
	
				LenPtext	=  16*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[13] 	= 	0xba637b39;
				Plaintext.u32[14] 	= 	0xaa0de657;
				Plaintext.u32[15] 	= 	0xb16aedf5;	
				
				LenAdata = 0;	
				
				LenIV = 3*4;
				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[1] = 0xfacedbad;
				InitVector.u32[2] = 0xcafebabe;				
				break;
	case 16:
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
				AesKey.u32[6]		= 	0x6d6a8f94;
				AesKey.u32[7]		= 	0x67308308;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 3*4;
				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[1] = 0xfacedbad;
				InitVector.u32[2] = 0xcafebabe;	
				break;
				
	case 17:
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
				AesKey.u32[6]		= 	0x6d6a8f94;
				AesKey.u32[7]		= 	0x67308308;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 2*4;
//				InitVector.u32[0] = 0xdecaf888;
				InitVector.u32[0] = 0xfacedbad;
				InitVector.u32[1] = 0xcafebabe;	
				break;
				
	case 18:
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
				AesKey.u32[6]		= 	0x6d6a8f94;
				AesKey.u32[7]		= 	0x67308308;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				LenAdata = 5*4;	
				AddAuthData.u32[0]		= 	0xdeadbeef;
				AddAuthData.u32[1]		= 	0xfeedface;
				AddAuthData.u32[2]		= 	0xdeadbeef;
				AddAuthData.u32[3]		= 	0xfeedface;
				AddAuthData.u32[4]		= 	0xabaddad2;
				
				LenIV = 15*4;
				InitVector.u32[0] = 0xff5269aa;
				InitVector.u32[1] = 0x55909c5a;
				InitVector.u32[2] = 0xf88406e5;
				InitVector.u32[3] = 0x9313225d;
				InitVector.u32[4] = 0xa318a728;
				InitVector.u32[5] = 0xe4c303d2;
				InitVector.u32[6] = 0x534f7da1;
				InitVector.u32[7] = 0x6a7a9538;
				InitVector.u32[8] = 0x9a6b5254;
				InitVector.u32[9] = 0xfcf0e242;
				InitVector.u32[10] = 0x56809539;
				InitVector.u32[11] = 0xc3c0c951;
				InitVector.u32[12] = 0xa637b39b;
				InitVector.u32[13] = 0xa0de6a57;
				InitVector.u32[14] = 0x16aedbf5;
				break;	
		default:
				break;
		}	
}

void CTR_TestCase_Init(void)
{
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
				AesKey.u32[6]		= 	0x6d6a8f94;
				AesKey.u32[7]		= 	0x67308308;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				CTR_Counter0.u32[0] = 0x06bc8eb4;
				CTR_Counter0.u32[1] = 0x079f8e24;
				CTR_Counter0.u32[2] = 0x140a5976;
				CTR_Counter0.u32[3] = 0x0cd953e2;
}

void CBC_TestCase_Init(void)
{
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
				AesKey.u32[6]		= 	0x6d6a8f94;
				AesKey.u32[7]		= 	0x67308308;
	
				LenPtext	=  15*4;
				Plaintext.u32[0]		= 	0xaff5269a;
				Plaintext.u32[1]		= 	0xa55909c5;
				Plaintext.u32[2]		= 	0xf88406e5;
				Plaintext.u32[3]		= 	0xd9313225;
				Plaintext.u32[4]		= 	0x8a318a72;
				Plaintext.u32[5]		= 	0x2e4c303d;
				Plaintext.u32[6]		= 	0x1534f7da;
				Plaintext.u32[7]		= 	0x86a7a953;
				Plaintext.u32[8] 		= 	0x49a6b525;
				Plaintext.u32[9] 		= 	0x2fcf0e24;
				Plaintext.u32[10] 	= 	0x95680953;
				Plaintext.u32[11] 	= 	0x1c3c0c95;
//				Plaintext.u32[12] 	= 	0x1aafd255;
				Plaintext.u32[12] 	= 	0xba637b39;
				Plaintext.u32[13] 	= 	0xaa0de657;
				Plaintext.u32[14] 	= 	0xb16aedf5;
			
				CTR_Counter0.u32[0] = 0xca6b0cba;
				CTR_Counter0.u32[1] = 0x52047196;
				CTR_Counter0.u32[2] = 0xf41a5514;
				CTR_Counter0.u32[3] = 0x83bcdd0a;
}
void ECB_TestCase_Init(void)
{
				AESKeyMode = KEY_Mode_256bits;
				AesKey.u32[0]		= 	0xfeffe992;
				AesKey.u32[1]		= 	0x8665731c;
				AesKey.u32[2]		= 	0x6d6a8f94;
				AesKey.u32[3]		= 	0x67308308;		
				AesKey.u32[4] 	= 	0xfeffe992;
				AesKey.u32[5] 	= 	0x8665731c;
				AesKey.u32[6]		= 	0x6d6a8f94;
				AesKey.u32[7]		= 	0x67308308;
	
				LenPtext	=  16*4;
				Plaintext.u32[0] 		= 	0x06bc8eb4;
				Plaintext.u32[1] 		= 	0x079f8e24;
				Plaintext.u32[2] 		= 	0x140a5976;
				Plaintext.u32[3] 		= 	0x0cd953e2;
				Plaintext.u32[4] 		= 	0x06bc8eb5;
				Plaintext.u32[5] 		= 	0x079f8e24;
				Plaintext.u32[6] 		= 	0x140a5976;
				Plaintext.u32[7] 		= 	0x0cd953e2;
				Plaintext.u32[8] 		= 	0x06bc8eb6;
				Plaintext.u32[9] 		= 	0x079f8e24;
				Plaintext.u32[10] 		= 	0x140a5976;
				Plaintext.u32[11] 		= 	0x0cd953e2;
				Plaintext.u32[12] 		= 	0x06bc8eb7;
				Plaintext.u32[13] 		= 	0x079f8e24;
				Plaintext.u32[14] 		= 	0x140a5976;
				Plaintext.u32[15] 		= 	0x0cd953e2;
}

#endif                                        /* This File Only support HT6x2x and HT501x */


#endif  /* __EXAMPLE_C */