library(plotly)
library(plyr)
library(ggplot2)

Sys.setenv("PATH" = paste(Sys.getenv("PATH"), "/home/carlos/anaconda3/bin/", sep = .Platform$path.sep))

##DEFINIÇÕES DOS EIXOS X e Y - (TAMANHO, FORMATO, E FONTE)
font_eixo <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 20,
  color = "black")

font_axis <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 20,
  color = "black")

# legenda <- list(
#   font = list(
#     #family = "Arial",
#     family = "Liberation Sans",
#     #family = "sans-serif",
#     size = 10,
#     color = "black"),
#   x=0.70,
#   y=0.99)#,
# #orientation = 'h')
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
  size = 16,
  color = "black")

##ENTRADA DE DADOS 
dados_2 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec50/50_2.txt", header = TRUE, sep="\t")
dados_3 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec50/50_3.txt", header = TRUE, sep="\t")
dados_5 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec50/50_5.txt", header = TRUE, sep="\t")

#Cálculo da porcentagem da taxa de detecção
tx_det_2 <- sum(dados_2$Det)/sum(dados_2$Na)*100
tx_det_3 <- sum(dados_3$Det)/sum(dados_3$Na)*100
tx_det_5 <- sum(dados_5$Det)/sum(dados_5$Na)*100

## Definie a sequência de dados
detec_data = c(tx_det_2, tx_det_3, tx_det_5)


## Forma do gráfico - (Formato, posição, tamanho, legenda)
rede <- c("2%", "5%", "10%")
ataq <- c(detec_data)

p <- plot_ly(x = ~rede, y = ~ataq,
             width=400, height=400,
             type = 'bar',
             name = '20%_ataque') %>%

  layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
         title = "(a) Rede com 50 nós ",  #chart title,
         #title = "Average number of communities", # chart title
         titlefont = title_font,
         #showlegend = TRUE, #legend = legenda,
         bargap=0.5)
p
orca(p, "detec_50.pdf") #linha para salvar o gráfico

  
  


