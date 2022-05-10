library(ggplot2)
library(plyr)
library(reshape2)
library(RColorBrewer)

#lendo a tabela
namefile_ag="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/ANALISES.txt"

#salvando o arquivo
nameMeanFile="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/imagem_1.pdf"
nameMeanFile_1="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/imagem_2.pdf"
#dados_10 = read.delim(namefile_ag, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")

#dados_1 <- readr::read_rds("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/ANALISES.txt")
dados_1 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/testes.txt", header = TRUE, sep="\t")
dados_2 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/testando.txt", header = TRUE, sep="\t")


# ggplot(dados_1, aes(x=time, y=ch)) +
#   ggtitle("Funcionamento") +
#   geom_line(colour = "darkblue")+
#   #geom_smooth()+
#   # geom_line(aes(y= as, color =  "Normal"))+
#   #geom_line(aes(y= a, color = "Ataque"))+
#   # geom_line(aes(y= cm, color = "Mecanismo"))+
#   xlab("Tempo (s)") +
#   ylab("Número de agrupamentos formados") +
#   theme(plot.title = element_text(hjust = 0.5))+
#   xlim(c(0, 1200)) +
#   ylim(c(0, 30))+
#   #scale_colour_manual("Séries")+
#   #theme_bw()+
#   ggsave(file=nameMeanFile,  width=10, height=4)

ggplot(dados_1, aes(x=time, y=ch)) +
  ggtitle("Funcionamento") +
  geom_line(colour = "darkblue")+
  #geom_smooth()+
  #geom_line(aes(y= as, color =  "Normal"))+
  #geom_line(aes(y= a, color = "Ataque"))+
  #geom_line(aes(y= cm, color = "Mecanismo"))+
  xlab("Tempo (s)") +
  ylab("Número de agrupamentos formados") +
  theme(plot.title = element_text(size=12, hjust = 0.5))+
  xlim(c(0, 1200)) +
  ylim(c(0, 30))+
  #scale_colour_manual("Séries")+
  #theme_bw()+
  ggsave(file=nameMeanFile_1, width=7, height=4)
  # ggplot(dados_1, aes(x=time, y=qosb))+
  #   ggtitle("Mesnsagens de controle")+
  #   geom_point(colour = "blue")+
  #   xlab("Tempo (s)") + 
  #   ylab("Numero de agrupamentos formados") + 
  #   theme(plot.title = element_text(hjust = 0.5))+
  #   xlim(c(0,1200)) +
  #   ylim(c(0,5500)) 
  # 
# + geom_smooth() # + geom_smooth(method=lm)
  
  
  # ggplot(pbr_mm, aes(x = index(pbr_mm))) + geom_line(aes(y = pbr_mm[,6], color = "PBR")) + 
  #   ggtitle("Série de preços da Petrobras") +
  #   geom_line(aes(y = pbr_mm$mm10, color = "MM10")) +
  #   geom_line(aes(y = pbr_mm$mm30, color = "MM30")) +
  #   xlab("Data") + ylab("Preço ($)") +
  #   theme(plot.title = element_text(hjust = 0.5), panel.border = element_blank()) +
  #   scale_x_date(date_labels = "%b %y", date_breaks = "3 months") +
  #   scale_colour_manual("Séries", values=c("PBR"="gray40", "MM10"="firebrick4", "MM30"="darkcyan"))
  
