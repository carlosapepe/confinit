  library(plotly)
  library(plyr)
  library(ggplot2)
  
  # Lab path
  namefile50="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/50nos.txt"
  namefile75="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/75nos.txt"
  namefile100="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulacoes/Dados/100nos.txt"
  #namefile75="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.53.txt"
  #namefile10="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.71.txt"
  #nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_p.png"
  #nameMeanFile="/home/asbatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/performance/mean_cois_6_e.png"
  nameMeanFile="/home/carlos/Documentos/Mestrado2019/Sbseg2019/Simulaoes/Dados/imagem_1"
  
  # Home path
  # namefile37="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.38.txt"
  # namefile52="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.53.txt"
  # namefile70="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/traces/stealth_global_Traces/Global_Coi_Trace_192.168.1.71.txt"
  # nameMeanFile="/Users/agnaldobatista/Dropbox/Mestrado/Orientação/Simulação/Graphs/Ver2/890s/Performance/meanCoIs.pdf"
  
  font_eixo <- list(
    #family = "Arial",
    family = "Liberation Sans",
    #family = "Courier New, monospace",
    size = 22,
    color = "black")
  
  font_axis <- list(
    #family = "Arial",
    family = "Liberation Sans",
    #family = "Courier New, monospace",
    size = 18,
    color = "black")
  
  legenda <- list(
    font = list(
      #family = "Arial",
      family = "Liberation Sans",
      #family = "sans-serif",
      size = 20,
      color = "black"),
      x=0.80,
      y=0.99)#,
    #orientation = 'h')
  eixo_x_1 <- list(
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
    range=c(0, 100),
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
    size = 22,
    color = "black")
  
  data50 <- read.delim(namefile50, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  data75 <- read.delim(namefile75, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  data100 <- read.delim(namefile100, header = TRUE, sep="\t", quote="", dec = ".", fill = FALSE, comment.char = "")
  
  #Leitura da primeiro coluna 50 nós
  data50_count <- c(1:length(data50$S)) # Contador do eixo x p/ plot 
  data50_data <- data.frame(data50_count, data50$S) # Merge dos dados p/ plot
  
  #leitura da segunda coluna 50 nós
  data50c_count <- c(1:length(data50$Na)) # Contador do eixo x p/ plot 
  data50c_data <- data.frame(data50c_count, data50$Na) # Merge dos dados p/ 
  
  #Leitura da primeiro coluna 75 nós
  data75_count <- c(1:length(data75$S)) # Contador do eixo x p/ plot 
  data75_data <- data.frame(data75_count, data75$S) # Merge dos dados p/ plot
  
  #Leitura da segunda coluna 75 nós
  data75c_count <- c(1:length(data75$Na)) # Contador do eixo x p/ plot 
  data75c_data <- data.frame(data75c_count, data75$Na) # Merge dos dados p/ plot
  
  #Leitura da primeiro coluna 100 nós 
  data100_count <- c(1:length(data100$S)) # Contador do eixo x p/ plot 
  data100_data <- data.frame(data100_count, data100$S) # Merge dos dados p/ plot
  
  #Leitura da primeiro coluna 100 nós 
  data100c_count <- c(1:length(data100$Na)) # Contador do eixo x p/ plot 
  data100c_data <- data.frame(data100c_count, data100$Na) # Merge dos dados p/ plot
  
  # # GEt mean from communities columns
   no50_mean = mean(data50$S)
   no50_sum = sum(data50$S)
   no50c_mean = mean(data50$Na)
   no50c_sum = sum(data50$Na)
   
   por50 <- (no50c_sum/no50_sum)*100
   
   no75_mean = mean(data75$S)
   no75_sum = sum(data75$S)
   no75c_mean = mean(data75$Na)
   no75c_sum = sum(data75$Na)
   
   
   por75 <- (no75c_sum/no75_sum)*100
   
   no100_mean = mean(data100$S)
   no100_sum = sum(data100$S)
   no100c_mean = mean(data100$Na)
   no100c_sum = sum(data100$Na)
   
   
   por100 <- (no100c_sum/no100_sum)*100
   
   
   #health52_mean = mean(data75$S)
   #health52_sum = sum(data75$S)
   
  # # music37_mean = mean(data37$nCoiC) 
  # # books37_mean = mean(data37$nCoiB) 
  # # tourism37_mean = mean(data37$nCoiT) 
  # # movies37_mean = mean(data37$nCoiM)
   
  # # music52_mean = mean(data52$nCoiC) 
  # # books52_mean = mean(data52$nCoiB) 
  # # tourism52_mean = mean(data52$nCoiT) 
  # # movies52_mean = mean(data52$nCoiM)
  # 
  # health70_mean = mean(data70$nCoiH) 
  # # music70_mean = mean(data70$nCoiC) 
  # # books70_mean = mean(data70$nCoiB) 
  # # tourism70_mean = mean(data70$nCoiT) 
  # # movies70_mean = mean(data70$nCoiM)
  # 
  # # #Define each data sequence to be put in chart
  #detec_data = c(por50, por75, por100)
  #detec_data = c(por50, por75) #por100)
  
  # # # music_data = c(music37_mean, music52_mean, music70_mean)
  # # # books_data = c(books37_mean, books52_mean, books70_mean)
  # # # tourism_data = c(tourism37_mean, tourism52_mean, tourism70_mean)
  # # # movies_data = c(movies37_mean, movies52_mean, movies70_mean)
  # # 
  
   rede <- c("50", "75", "100")
   ataq_20 <- c(90, 94, 95)
   ataq_30 <- c(92, 95, 96)
   data <- data.frame(Animals, ataq_20, ataq_30)
   
   p <- plot_ly(data, x = rede, y = ataq_20, type = 'bar', name = '30%_ataque') %>%
     add_trace(y = ataq_30, name = '20%_ataque') %>%
  
   # eixoxx <- c("50","75","100")
   # ataque_20 <- c(90,94,95)
   # ataque_30 <- c(92,93,94)
   # data_1 <- data.frame(eixoxx,ataque_20,ataque_30)
   # 
   # 
   # p <- plot_ly(data_1, x = eixoxx, y =
   #              x = c("50","75","100"), 
   #              y = c(90,94,95),
   #              width=500, height=500,  #Chart aspect ratio
   #              name = "", 
   #              type = "bar") %>%
   #  
                 #list(color = 'rgb(158,202,225)',
      #name = "Health Communities", type = "bar") %>%
      layout(xaxis = eixo_x_1, yaxis = eixo_y_1, 
             title = "(a) 20% de Ataque IDF ", # chart title
             #title = "Average number of communities", # chart title
             #titlefont = title_font,
             #showlegend = TRUE, legend = legenda,
             bargap=0.5)
    
   #p
   #export(p, file = nameMeanFile)