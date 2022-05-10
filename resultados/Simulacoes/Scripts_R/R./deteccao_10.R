library(plotly)
library(plyr)
library(ggplot2)

Sys.setenv("PATH" = paste(Sys.getenv("PATH"), "/home/carlos/anaconda3/bin/", sep = .Platform$path.sep))
# Lab path
namefile50_10="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/50nos_10.txt"
namefile75_10="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/75nos_10.txt"
namefile100_10="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/100nos_10.txt"
#namefile75="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.53.txt"
#namefile10="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.71.txt"
#nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_p.png"
#nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_e.png"
nameMeanFile="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/imagem_1.png"

# Home path
# namefile37="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.38.txt"
# namefile52="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.53.txt"
# namefile70="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.71.txt"
# nameMeanFile="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/Performance/meanCoIs.pdf"

font_eixo <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 28,
  color = "black")

font_axis <- list(
  #family = "Arial",
  family = "Liberation Sans",
  #family = "Courier New, monospace",
  size = 28,
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
  title = "Número de Nós na Rede",
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
  dtick = (10),
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
  size = 20,
  color = "black")

data50_10 <- read.delim(namefile50_10, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
data75_10 <- read.delim(namefile75_10, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
data100_10 <- read.delim(namefile100_10, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")

#Leitura da primeiro coluna 50 nós
data50_10_count <- c(1:length(data50_10$S)) # Contador do eixo x p/ plot 
data50_data_10 <- data.frame(data50_10_count, data50_10$S) # Merge dos dados p/ plot

#leitura da segunda coluna 50 nós
data50_10c_count <- c(1:length(data50_10$Na)) # Contador do eixo x p/ plot 
data50c_10_data <- data.frame(data50c_count, data50_10$Na) # Merge dos dados p/ 

#Leitura da primeiro coluna 75 nós
data75_10_count <- c(1:length(data75_10$S)) # Contador do eixo x p/ plot 
data75_10_data <- data.frame(data75_10_count, data75_10$S) # Merge dos dados p/ plot

#Leitura da segunda coluna 75 nós
data75c_10_count <- c(1:length(data75_10$Na)) # Contador do eixo x p/ plot 
data75c_10_data <- data.frame(data75c_count, data75_10$Na) # Merge dos dados p/ plot

#Leitura da primeiro coluna 100 nós 
data100_10_count <- c(1:length(data100_10$S)) # Contador do eixo x p/ plot 
data100_10_data <- data.frame(data100_10_count, data100_10$S) # Merge dos dados p/ plot

#Leitura da primeiro coluna 100 nós 
data100c_10_count <- c(1:length(data100$Na)) # Contador do eixo x p/ plot 
data100c_10_data <- data.frame(data100c_10_count, data100_10$Na) # Merge dos dados p/ plot

# # GEt mean from communities columns
no50_10_mean = mean(data50_10$S)
no50_10_sum = sum(data50_10$S)
no50c_10_mean = mean(data50_10$Na)
no50c_10_sum = sum(data50_10$Na)

por50_10 <- (no50c_10_sum/no50_10_sum)*100

no75_10_mean = mean(data75_10$S)
no75_10_sum = sum(data75_10$S)
no75c_10_mean = mean(data75_10$Na)
no75c_10_sum = sum(data75_10$Na)

por75_10 <- (no75c_10_sum/no75_10_sum)*100

no100_10_mean = mean(data100_10$S)
no100_10_sum = sum(data100_10$S)
no100c_10_mean = mean(data100_10$Na)
no100c_10_sum = sum(data100_10$Na)

por100_10 <- (no100c_sum/no100_sum)*100

# # #Define each data sequence to be put in chart
detec_data = c(por50_10, por75_10, por100_10)
#detec_data = c(por50, por75) #por100)


rede <- c("50", "75", "100")
ataq_20 <- c(detec_data)
ataq_20 <- c(92, 95, 96)
data <- data.frame(rede, ataq_20)

p <- plot_ly(data =  x = ~rede, y = ~ataq_20,
             width=500, height=500,
             type = 'bar',
             name = '20%_ataque') %>%
  #layout(xaxis = xform)

  layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
         #title = "(a) 10% de ataque IDF ",  #chart title,
         #title = "Average number of communities", # chart title
         titlefont = title_font,
         showlegend = TRUE, #legend = legenda,
         bargap=0.5)
p
orca(p, "detec_10.png") 
  
  
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

