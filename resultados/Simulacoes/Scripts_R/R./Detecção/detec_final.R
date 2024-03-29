library(plotly)
library(plyr)
library(ggplot2)

Sys.setenv("PATH" = paste(Sys.getenv("PATH"), "/home/carlos/anaconda3/bin/", sep = .Platform$path.sep))

##DEFINIÇÕES DOS EIXOS X e Y - (TAMANHO, FORMATO, E FONTE)
font_eixo <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 24,
  color = "black")

font_axis <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 24,
  color = "black")

legenda <- list(
  font = list(
    #family = "Arial",
    family = "Liberation Sans",
    #family = "sans-serif",
    size = 18,
    color = "black"),
  x=0.60,
  y=0.07)#,
#orientation = 'h')
eixo_x_1 <- list(
  categoryorder = "array",
  categoryarray = c("50", "75", "100"),
  title = "(%) de ataque na rede",
  #title = "Nodes",
  tick0 = 0,
  tickwidth = 2,
  #dtick = 100,
  tickfont = font_axis,
  titlefont = font_eixo,
  mirror=TRUE,
  ticks='outside',
  showline=TRUE,
  gridcolor = toRGB("lightgray"),
  gridwidth = 1,
  showgrid = FALSE)
eixo_y_1 <- list(
  title = "Taxa de Detecção (%)",
  #title = "Number of Communities",  
  dtick = (20),
  tickwidth = 2,
  range=c(0, 110),
  showline=TRUE,
  tick0 = 0,
  tickfont = font_axis,
  titlefont = font_eixo,
  mirror=TRUE,
  ticks='outside',
  showline=TRUE,
  gridcolor = toRGB("lightgray"),
  gridwidth = 1,
  showgrid = TRUE)

title_font <- list(
  #family = "Arial",
  family = "Liberation Sans",
  size = 24,
  color = "black")

##ENTRADA DE DADOS 50 NÓS 
dados_50_2 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec50/50_2.txt", header = TRUE, sep="\t")
dados_50_3 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec50/50_3.txt", header = TRUE, sep="\t")
dados_50_5 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec50/50_5.txt", header = TRUE, sep="\t")

##ENTRADA DE DADOS 75 NÓS
dados_75_2 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec75/75.2.txt", header = TRUE, sep="\t")
dados_75_4 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec75/75_4.txt", header = TRUE, sep="\t")
dados_75_8 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec75/75_8.txt", header = TRUE, sep="\t")

#ENTRADA DE DADOS 100 NÓS
dados_100_2 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec100/100_2.txt", header = TRUE, sep="\t")
dados_100_5 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec100/100_5.txt", header = TRUE, sep="\t")
dados_100_10 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec100/100_10.txt", header = TRUE, sep="\t")


#Cálculo da porcentagem da taxa de detecção 50 nós
tx_det_50_2 <- sum(dados_50_2$Det)/sum(dados_50_2$Na)*100
tx_det_50_3 <- sum(dados_50_3$Det)/sum(dados_50_3$Na)*100
tx_det_50_5 <- sum(dados_50_5$Det)/sum(dados_50_5$Na)*100

#Cálculo da porcentagem da taxa de detecção 100 nós
tx_det_100_2 <- sum(dados_100_2$Det)/sum(dados_100_2$Na)*100
tx_det_100_5 <- sum(dados_100_5$Det)/sum(dados_100_5$Na)*100
tx_det_100_10 <- sum(dados_100_10$Det)/sum(dados_100_10$Na)*100

#Cálculo da porcentagem da taxa de detecção 75 nós
tx_det_75_2 <- sum(dados_75_2$Det)/sum(dados_75_2$Na)*100
tx_det_75_4 <- sum(dados_75_4$Det)/sum(dados_75_4$Na)*100
tx_det_75_8 <- sum(dados_75_8$Det)/sum(dados_75_8$Na)*100


## Definie a sequência de dados 50
detec_data_50 = c(tx_det_50_2, tx_det_50_3, tx_det_50_5)
## Define a sequência de dados 75 
detec_data_75 = c(tx_det_75_2, tx_det_75_4, tx_det_75_8)
## Define a sequência de dados 100
detec_data_100 = c(tx_det_100_2, tx_det_100_5, tx_det_100_10)

# Forma do gráfico - (Formato, posição, tamanho, legenda)
rede <- c("2%", "5%", "10%")
ataq <- c(detec_data_50)
ataq_1 <- c(100, 99.00, 98.00)
ataq_2 <- c(detec_data_100)
data <- data.frame(rede, ataq, ataq_2)

p <- plot_ly(x = ~rede, y = ~ataq,
             width=500, height=500,
             type = 'bar', marker = list(color = 'rgb(0.0,0.5,0.0)'),
             name = '50_nós') %>%
  add_trace(y = ~ataq_1, name = '75_nós',
            marker = list(color = 'rgb(1.0,0.0,0.0')) %>%
  add_trace(y = ~ataq_2, name = '100_nós',
            marker = list(color = 'rgb(0.0,0.0,1.0)')) %>%

  layout(xaxis = eixo_x_1, yaxis = eixo_y_1,
         title = "(a)",  #chart title,
         #title = "Average number of communities", # chart title
         titlefont = title_font,
         showlegend = TRUE, legend = legenda,
         bargap=0.4)
p
orca(p, "detec_total.pdf") #linha para salvar o gráfico
















