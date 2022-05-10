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

dados_2 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec75/75_2.txt", header = TRUE, sep="\t")
dados_4 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec75/75_4.txt", header = TRUE, sep="\t")
dados_8 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/detec75/75_8.txt", header = TRUE, sep="\t")


tx_det_2 <- sum(dados_2$Det)/sum(dados_2$Na)*100
tx_det_4 <- sum(dados_4$Det)/sum(dados_4$Na)*100
tx_det_8 <- sum(dados_8$Det)/sum(dados_8$Na)*100

# # #Define each data sequence to be put in chart
detec_data_75 = c(tx_det_2, tx_det_4, tx_det_8)

rede <- c("2%", "5%", "10%")
ataq <- c(detec_data_75)

p <- plot_ly(x = ~rede, y = ~ataq,
             width=400, height=400,
             type = 'bar',
             name = '20%_ataque') %>%
  #layout(xaxis = xform)

  layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
         title = "(b) Rede com 75 nós ",  #chart title,
         #title = "Average number of communities", # chart title
         titlefont = title_font,
         #showlegend = TRUE, #legend = legenda,
         bargap=0.5)
p
orca(p, "detec_75.pdf") 
  
  
  #add_trace(y = ~ataq_30, name = '30%_ataque') %>%
#   
# p <- plot_ly(
#   x = c("50","75","99"), 
#   #y = c(90,94,95),
#   #y = c(detec_data),
#   y  = c(12,17,23),
#   #marker = list(color = 'rgb(55, 83, 109)'), 
#   #marker = list(color = 'rgba(204,204,204,1)'),
#   width=500, height=500,  #Chart aspect ratio
#   #name = "", 
#   type = "bar") %>%
#   #list(color = 'rgb(158,202,225)',
#   #name = "Health Communities", type = "bar") %>%
  # layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
  #        title = "(a) 10% de ataque IDF ",  #chart title,
  #        #title = "Average number of communities", # chart title
  #        #titlefont = title_font,
  #        #showlegend = TRUE, #legend = legenda,
  #        bargap=0.5)

#

