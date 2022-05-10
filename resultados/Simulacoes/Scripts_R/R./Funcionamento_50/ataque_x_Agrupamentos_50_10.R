library(plotly)
library(plyr)
library(ggplot2)

Sys.setenv("PATH" = paste(Sys.getenv("PATH"), "/home/carlos/anaconda3/bin/", sep = .Platform$path.sep))

##DEFINIÇÕES DOS EIXOS X e Y - (TAMANHO, FORMATO, E FONTE)
font_eixo <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 18, #16
  color = "black")

font_axis <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 18, #24,
  color = "black")

legenda <- list(
  font = list(
    #family = "Arial",
    family = "Liberation Sans",
    #family = "sans-serif",
    size = 18, #16,
    color = "black"),
  #x=0.75,
  x=0.60,
  y=0.99)#,
#orientation = 'h')
eixo_x_1 <- list(
  title = "Tempo (s)",
  #title = "Time (s)",
  tick0 = 0,
  #tickwidth = 2,
  range=c(0, 1300),
  dtick = 200,
  showticklabels = TRUE,
  tickfont = font_axis,
  titlefont = font_eixo,
  mirror=TRUE,
  #ticks='outside',
  showline=TRUE,
  gridcolor = toRGB("lightgray"),
  gridwidth = 1,
  showgrid = TRUE)
eixo_y_1 <- list(
  #title = "Número de Agrupamentos Formados",
  title = "Agrupamentos formados",  
  dtick = 10,
  tickwidth = 5,
  range=c(0, 30),
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
  size = 19,
  color = "black")

NodeFont <- list(
  family = "Liberation Sans",
  size = 14,
  color = "black")

nodeName <- list(
  #font = list(size = 12);
  x = 830,
  y = 12,
  text = "Nó 37",
  #xref = "x",
  #yref = "y",
  showarrow = FALSE,
  arrowhead = 12,
  xanchor = "left",
  font = NodeFont
  )

##ENTRADA DOS DADOS
dados_10 <- read.delim("/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/50nos/dados_50_10.txt", header = TRUE, sep="\t")


#Cálculo da média de formação dos agrupamentos
media_ch_10 <- mean(dados_10$ch)
media_at_10 <- mean(dados_10$at)
media_cm_10 <- mean(dados_10$cm)



#configurações dos gráficos - GRÁFICO DE LINHAS  
p <- plot_ly(x = dados_10$time, y = dados_10$ch, name = 'DDFC',
                 width=500, height=300, # Chart aspect ratio
                 type = 'scatter', mode = 'line+markers', marker = list(symbol = 22, size = 2, color = 'blue'),
                 line=list(color='blue', width=1))%>%
                 #type = 'scatter', mode = 'lines', line = list(width = 4, color = 'blue')) %>%
                 add_trace(y = dados_10$at, name = 'DDFC+IDF',
                              type = 'scatter', mode = 'line+markers', marker = list(symbol = 31, size = 2, color = 'red'),
                              line=list(color='red', width=1, dash='dot'))%>%
                              #line = list(color = 'red', dash = 'dash')) %>%
               add_trace(y = dados_10$cm, name = 'CONF+IDF',
                          type = 'scatter', mode = 'line+markers', 
                          marker = list(symbol = 2, size = 2, color = 'green'),
                          line=list(color='green', width=1, dash='dot'))%>%
                          #line = list(color = 'red', dash = 'dash')) %>%
  
  layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
         title = "(c) 50 nós 10% de atacantes", # chart title
         #title = "Average number of communities", # chart title
         titlefont = title_font,
         showlegend = TRUE, legend = legenda,
         bargap=1)

   # layout(xaxis = eixo_x_1, yaxis = eixo_y_1,
   #        title = "Nó 37", # chart title
   #        title = "Evolution of health communities", # chart title
   #        titlefont = title_font,
   #        showlegend = TRUE, legend = legenda, annotations = nodeName)
p
orca(p, "50_10.pdf")