library(plyr)
library(reshape2)
library(ggplot2)
library(RColorBrewer) #colorpalletes
library(micEcon) #insertRow
library(XML)

aes_now <- function(...) {
  structure(list(...),  class = "uneval")
}

## Summarizes data.
## Gives count, mean, standard deviation, standard error of the mean, and confidence interval (default 95%).
##   data: a data frame.
##   measurevar: the name of a column that contains the variable to be summariezed
##   groupvars: a vector containing names of columns that contain grouping variables
##   na.rm: a boolean that indicates whether to ignore NA's
##   conf.interval: the percent range of the confidence interval (default is 95%)
summarySE <- function(data=NULL, measurevar, groupvars=NULL, na.rm=FALSE, conf.interval=.95, .drop=TRUE) {
	require(plyr)

	# New version of length which can handle NA's: if na.rm==T, don't count them
	length2 <- function (x, na.rm=FALSE) {
		if (na.rm) sum(!is.na(x))
		else       length(x)
	}

	# This is does the summary; it's not easy to understand...
	datac <- ddply(data, groupvars, .drop=.drop,
		.fun= function(xx, col, na.rm) {
		c( N    = length2(xx[,col], na.rm=na.rm),
		mean = mean   (xx[,col], na.rm=na.rm),
		sd   = sd     (xx[,col], na.rm=na.rm)
		)
		},
		measurevar,
		na.rm
	)

	# Rename the "mean" column    
	datac <- rename(datac, c("mean"=measurevar))

	datac$se <- datac$sd / sqrt(datac$N)  # Calculate standard error of the mean

	# Confidence interval multiplier for standard error
	# Calculate t-statistic for confidence interval: 
	# e.g., if conf.interval is .95, use .975 (above/below), and use df=N-1
	ciMult <- qt(conf.interval/2 + .5, datac$N-1)
	datac$ci <- datac$se * ciMult

	return(datac)
}

# ENTIRE TABLE
f <- read.table("/Users/fgielow/Dropbox/FACUL/Dissertacao/SIM_ANALYSIS/KHOPCA/GLOBAL_TABLE_FOR_R_2", row.names=NULL, header=TRUE)
basepath="/Users/fgielow/Dropbox/FACUL/Dissertacao/SIM_ANALYSIS/KHOPCA/PDFs/"

beacons_vs_time_ch_cthresh <- function() {


	INT_PARAMS <- c("p6", "p12", "p24", "p48", "p96")
	ADAP_PARAMS <- c("p0", "p1")
	MK_PARAMS <- c("p1", "p2", "p3")

	for (pint in INT_PARAMS) {
		for (padap in ADAP_PARAMS) {
			for (pmk in MK_PARAMS) {

				#FILENAME
				filename=paste(basepath,"beacons_vs_time_ch_cthresh",sep="")
				filename=paste(filename,"adap",sep=":")
				filename=paste(filename,substring(padap,2),sep=":")
				filename=paste(filename,"int",sep=":")
				filename=paste(filename,substring(pint,2),sep=":")
				filename=paste(filename,"mk",sep=":")
				filename=paste(filename,substring(pmk,2),sep=":")
				filename=paste(filename,"pdf",sep=".")

				titulo="Quantidade de mensagens beacon enviadas para "
				if (padap=="p1") {
				titulo=paste(titulo,"intervalo adaptativo\n",sep="")
				} else {
					titulo=paste(titulo,"intervalo não adaptativo\n",sep="")
				}
				titulo=paste(titulo,"com intervalo-base = ",sep="")
				titulo=paste(titulo,substring(pint,2),sep="")
				titulo=paste(titulo,".0s",sep="")
				titulo=paste(titulo,", maxK = ",sep="")
				titulo=paste(titulo,substring(pmk,2),sep="")


				# SELECT PARAMETERS
				f1 <- f[f$int==pint & f$adap==padap & f$mk==pmk,]

				# GET SUMMARY
				f2 <- summarySE(f1, measurevar="qosb", groupvars=c("time","cth"))

				# AVOID NAs
				f2$ci[is.na(f2$ci)]=0

				# TIME INTERVAL
				f3 <- f2[f2$time%%50==0 | f2$time==1190 | f2$time==10,]
				# f3 <- f2[(f2$cth=="p0.5" & f2$time%%80==0) | (f2$cth=="p1.0" & (f2$time+20)%%80==0) | (f2$cth=="p1.5" & (f2$time+40)%%80==0) | (f2$cth=="p2.0" & (f2$time+60)%%80==0) | f2$time==1190 | f2$time==10,]

				# The errorbars overlapped, so use position_dodge to move them horizontally
				pd <- position_dodge(width=.1) # move them to the left and right

				ggplot(f3, aes(x=time, y=qosb,colour=cth, group=cth, shape=cth)) +
				geom_errorbar(aes(ymin=qosb-ci, ymax=qosb+ci), position=pd) +
				geom_line(position=pd) +
				geom_point(position=pd, size=3) +
				scale_colour_discrete() +
				scale_shape_manual(values=as.numeric(f3$cth)) +
				xlab("Tempo") +
				ylab("Número de mensagens Beacon enviadas até o momento") +
				scale_colour_hue(name="Threshold de\nsimilaridade", # Legend label, use darker colors
				breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
				labels=c("0.5", "1.0", "1.5", "2.0"),
				l=40) +                  # Use darker colors, lightness=40
				scale_shape_discrete(name="Threshold de\nsimilaridade", # Legend label, use darker colors
				breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
				labels=c("0.5", "1.0", "1.5", "2.0")) +
				opts(title=titulo) +
				#scale_y_continuous(limits=c(0, max(f2$qosb + f2$ci)),    # Set y range
				#                   breaks=0:16*200) +                       # Set tick every 100
				scale_y_continuous(expand=c(0,0), limits=c(0, max(f$qosb))) + #higuest possible value in all data
				theme_bw() +
				opts(
					#legend.justification=c(1,0),
					#legend.position=c(0.9,0.1), 
					legend.justification=c(0,1),
					legend.position=c(0.16,0.89),
					legend.key = theme_blank()) + 
					opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
					opts(aspect.ratio=1/1) +
					opts(axis.title.x = theme_text(vjust=-0.1)) + # more space below Xlabel
					ggsave(file=filename)
			}
		}
	}
}


ir_vs_time_ch_adap <- function() {


	INT_PARAMS <- c("p6", "p12", "p24", "p48", "p96")
	CTH_PARAMS <- c("p0.5", "p1.0", "p1.5", "p2.0")
	MK_PARAMS <- c("p1", "p2", "p3")

	# maxIR=max(f$ir)
	
	for (pint in INT_PARAMS) {
		for (pcth in CTH_PARAMS) {
			for (pmk in MK_PARAMS) {

				#FILENAME
				filename=paste(basepath,"ir_vs_time_ch_adap",sep="")
				filename=paste(filename,"cth",sep=":")
				filename=paste(filename,substring(pcth,2),sep=":")
				filename=paste(filename,"int",sep=":")
				filename=paste(filename,substring(pint,2),sep=":")
				filename=paste(filename,"mk",sep=":")
				filename=paste(filename,substring(pmk,2),sep=":")
				filename=paste(filename,"pdf",sep=".")

				titulo="Quantidadede de rotas inconsistentes considerando\n"
				titulo=paste(titulo,"intervalo-base = ",sep="")
				titulo=paste(titulo,substring(pint,2),sep="")
				titulo=paste(titulo,".0s",sep="")
				titulo=paste(titulo,", maxK = ",sep="")
				titulo=paste(titulo,substring(pmk,2),sep="")
				titulo=paste(titulo,", e cThresh = ",sep="")
				titulo=paste(titulo,substring(pcth,2),sep="")
				
				# SELECT PARAMETERS
				f1 <- f[f$int==pint & f$cth==pcth & f$mk==pmk,]

				# GET SUMMARY
				f2 <- summarySE(f1, measurevar="ir", groupvars=c("time","adap"))

				# AVOID NAs
				f2$ci[is.na(f2$ci)]=0

				# TIME INTERVAL
				f3 <- f2[f2$time%%50==0 | f2$time==1190 | f2$time==10,]

				# The errorbars overlapped, so use position_dodge to move them horizontally
				pd <- position_dodge(width=.1) # move them to the left and right


				ggplot(f3, aes(x=time, y=ir,colour=adap, group=adap, shape=adap)) +
				    geom_errorbar(aes(ymin=ir-ci, ymax=ir+ci), position=pd) +
				    geom_line(position=pd) +
				    geom_point(position=pd, size=3) +
				    scale_colour_discrete() +
				    scale_shape_manual(values=as.numeric(f3$adap)) +
				    xlab("Tempo") +
				    ylab("Número de rotas inconsistentes") +
				    scale_colour_hue(name="Intervalo", # Legend label, use darker colors
				                     breaks=c("p0", "p1"),
				                     labels=c("não adaptativo", "adaptativo"),
				                     l=40) +                  # Use darker colors, lightness=40
				    scale_shape_discrete(name="Intervalo", # Legend label, use darker colors
				                         breaks=c("p0", "p1"),
					                     labels=c("não adaptativo", "adaptativo")) +
				    opts(title=titulo) +
				    scale_y_continuous(expand=c(0,0), limits=c(-0.2, 3)) +
				    theme_bw() +
				    opts(
						legend.justification=c(1,1),
						legend.position=c(0.89,0.89),
				        legend.key = theme_blank()) + 
						opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
				    	opts(aspect.ratio=1/1) +
						opts(axis.title.x = theme_text(vjust=-0.1)) + # more space below Xlabel
					ggsave(file=filename)
					
					
					
					# no lugar de adap é a coluna descritora da parada

					# ggplot (f3, aes(time, reorder(adap,ir,mean))) +
					#   geom_point(position = position_jitter(height = 0.025), alpha = 0.3, aes(size=ir))
					
			}
		}
	}
}


ir_vs_time_scatterplot <- function() {


	INT_PARAMS <- c("p6", "p12", "p24", "p48", "p96")
	CTH_PARAMS <- c("p0.5", "p1.0", "p1.5")
	# MK_PARAMS <- c("p1", "p2", "p3")
	ADAP_PARAMS <-c("p1", "p0")
	
	desired_order <- c()
	
	col_names <- c("time", "adap", "N", "ir", "sd", "se", "ci", "params")
	f3 <- data.frame(rbind(c(NA,NA,NA,NA,NA,NA,NA,NA)))
	names(f3) <- col_names
	f3 <- f3[-1,] # remove NAs
	
	
	filename=paste(basepath,"scatterplot_ir.pdf",sep="")

	for (padap in ADAP_PARAMS) {
		for (pint in INT_PARAMS) {
			for (pcth in CTH_PARAMS) {
				# for (pmk in MK_PARAMS) {
				
				
					# SELECT PARAMETERS
					# f1 <- f[f$adap==padap & f$int==pint & f$cth==pcth & f$mk==pmk,]
					f1 <- f[f$adap==padap & f$int==pint & f$cth==pcth,]

					# GET SUMMARY
					f2 <- summarySE(f1, measurevar="ir", groupvars=c("time"))

					# AVOID NAs
					f2$ci[is.na(f2$ci)]=0
				
					cname=""
					cname=paste(cname,"CThresh ",sep="")
					cname=paste(cname,substring(pcth,2),sep="")
					cname=paste(cname,", int ",sep="")
					cname=paste(cname,substring(pint,2),sep="")
					cname=paste(cname,", adap ",sep="")
					cname=paste(cname,substring(padap,2),sep="")
					
					print(cname)
					
					desired_order <- c(desired_order,cname)
					f2$params <- cname
				
					f3 <- merge(f3,f2,all=TRUE,sort=TRUE)
				# }
			}
		}
	}
	
	f3 <- f3[f3$ir>0.0,]
	
	f3$params  <- factor( as.character(f3$params), levels=desired_order )
	f3 <- f3[order(f3$params),]

	# no lugar de adap é a coluna descritora da parada

	x_axis_params_ir <- factor(f3$params)
	
	ggplot (f3, aes_now(x=x_axis_params_ir, y=f3$time)) +
	geom_point(position = position_jitter(width = 0.1), alpha = 0.35, aes(size=ir, color=ci)) + 
	ylab("Tempo") +
	xlab("Parâmetros") +
	scale_size_continuous(name="Rotas\ninconsistentes") +
	scale_color_continuous(name="Intervalo de\nconfiança") +
	opts(aspect.ratio=1/3) + 
	opts(axis.text.x=theme_text(angle=-60, hjust=0,vjust=1)) +
	ggsave(file=filename,  width=11.75, height=5.14) # aspect ratio was not working, wtf
	
}


beacon_vs_time_scatterplot <- function() {


	INT_PARAMS <- c("p6", "p12", "p24", "p48", "p96")
	CTH_PARAMS <- c("p0.5", "p1.0", "p1.5")
	MK_PARAMS <- c("p1", "p2", "p3")
	ADAP_PARAMS <-c("p0", "p1")

	desired_order <- c()

	col_names <- c("time", "adap", "N", "qosb", "sd", "se", "ci", "params")
	f3 <- data.frame(rbind(c(NA,NA,NA,NA,NA,NA,NA,NA)))
	names(f3) <- col_names
	f3 <- f3[-1,] # remove NAs


	filename=paste(basepath,"beacon_vs_time_scatterplot.pdf",sep="")

	for (padap in ADAP_PARAMS) {
		for (pint in INT_PARAMS) {
			for (pmk in MK_PARAMS) {
				# for (pmk in MK_PARAMS) {


					# SELECT PARAMETERS
					# f1 <- f[f$adap==padap & f$int==pint & f$cth==pcth & f$mk==pmk,]
					f1 <- f[f$adap==padap & f$int==pint & f$mk==pmk,]
					names(f1)[names(f1)=="sd"] <- "seed"
					f1 <- summarySE(f1, measurevar="qosb", groupvars=c("time","adap","int","mk", "seed"))

					# ORDER BY TIME! :)
					f1 <-  f1[with(f1,order(time)),]

					col_names <- c("time", "adap", "N", "qosb", "seed", "se", "ci", "params")
					f1m <- data.frame(rbind(c(NA,NA,NA,NA,NA,NA,NA,NA)))
					names(f1m) <- col_names
					f1m <- f1m[-1,] # remove NAs
					
					# UNACUMULATE FUCKING QOSBS
					for (sd in 1:35) {
						f11 <- f1[f1$seed == sd,]
						v <- f11$qosb
						for (i in length(v):2 ) { v[i] <- v[i] - v[i-1] }
						f11$qosb <- v
						f1m <- merge(f1m,f11,all=TRUE,sort=TRUE)
					}


					# GET SUMMARY
					f2 <- summarySE(f1m, measurevar="qosb", groupvars=c("time"))

					# AVOID NAs
					f2$ci[is.na(f2$ci)]=0

					cname=""
					cname=paste(cname,"MaxK ",sep="")
					cname=paste(cname,substring(pmk,2),sep="")
					cname=paste(cname,", int ",sep="")
					cname=paste(cname,substring(pint,2),sep="")
					cname=paste(cname,", adap ",sep="")
					cname=paste(cname,substring(padap,2),sep="")

					print(cname)

					desired_order <- c(desired_order,cname)
					f2$params <- cname

					f3 <- merge(f3,f2,all=TRUE,sort=TRUE)
				# }
			}
		}
	}

	f3<-f3[f3$qosb>0,] # no zeros appearing -.-
	
	# f3$cth <- factor(f3$cth)
	
	f3$params  <- factor( as.character(f3$params), levels=desired_order )
	f3 <- f3[order(f3$params),]

	# no lugar de adap é a coluna descritora da parada

	x_axis_params_qosb <- factor(f3$params)

	ggplot (f3, aes_now(x=x_axis_params_qosb, y=f3$time)) +
	geom_point(position = position_jitter(width = 0.1), alpha = 0.35, aes(size=qosb, color=ci)) + 
	ylab("Tempo") +
	xlab("Parâmetros") +
	scale_size_continuous(name="Beacons\nenviados") +
	scale_color_continuous(name="Intervalo de\nconfiança") +
	opts(aspect.ratio=1/3) + 
	opts(axis.text.x=theme_text(angle=-50, hjust=0,vjust=1)) +
	ggsave(file=filename,  width=11.75, height=5.14) # aspect ratio was not working, wtf
	
}


ir_vs_mk_ch_int <- function() {

	CTH_PARAMS <- c("p0.5", "p1.0", "p1.5", "p2.0")
	ADAP_PARAMS <- c("p0", "p1")
	

	#FILENAME
	filename=paste(basepath,"ir_vs_mk_ch_int.pdf",sep="")
	
	f1 <- f

	f2 <- summarySE(f1, measurevar="ir", groupvars=c("int","mk", "cth", "adap"))
	f2$ci[is.na(f2$ci)]=0
	desired_order <- c("p6", "p12", "p24", "p48", "p96")
	f3 <- f2
	f3$int  <- factor( as.character(f3$int), levels=desired_order )
	f3 <- f3[order(f3$int),]

	colours <- colorRampPalette(brewer.pal(6,"Greys"))(6)
	dodge <- position_dodge(width = 0.9)



	#rename everything
	levels(f3$adap) <- gsub("p0", "intervalo não\nadaptativo", levels(f3$adap))
	levels(f3$adap) <- gsub("p1", "intervalo\nadaptativo", levels(f3$adap))

	levels(f3$cth) <- gsub("p0.5", "CThresh = 0.5", levels(f3$cth))
	levels(f3$cth) <- gsub("p1.0", "CThresh = 1.0", levels(f3$cth))
	levels(f3$cth) <- gsub("p1.5", "CThresh = 1.5", levels(f3$cth))
	levels(f3$cth) <- gsub("p2.0", "CThresh = 2.0", levels(f3$cth))

	ggplot(f3, aes(x=mk, y=ir, fill=int)) + 
	facet_grid(adap ~ cth) + 
	geom_bar(position=dodge, colour="black") + 
	geom_errorbar(width=.3, position=dodge, aes(ymin=ir-ci, ymax=ir+ci)) + 
	opts(title="") +
	xlab("maxK") +
	ylab("Média acumulada de rotas inconsistentes") +
	scale_fill_manual(values=colours, 
		name="Intervalo\nbase",
		breaks=c("p6", "p12", "p24", "p48", "p96"), 
		labels=c("6.0s", "12.0s", "24.0s", "48.0s", "96.0s") ) +
	scale_x_discrete(breaks=c("p1", "p2", "p3"),
					   labels=c("1","2","3")) +
	scale_y_continuous(expand=c(0,0), limits=c(0,1.4)) +
	theme_bw() +
	   opts(
		legend.justification=c(0,1),
		legend.position="right",
	       legend.key = theme_blank()) + 
	# opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
	opts(axis.title.x = theme_text(vjust=-0.1)) + # more space below Xlabel
	opts(aspect.ratio=1/1.5) +
	ggsave(file=filename, width=10.6,height=4)

}

ch_vs_cth_ch_mk <- function () {
	
	filename=paste(basepath,"ch_vs_cth_ch_mk.pdf",sep="")
	# titulo="Número de líderes na rede considerando diferentes maxK"
	titulo=""
	
	f1 <- f	
	f2 <- summarySE(f1, measurevar="ch", groupvars=c("cth","mk"))
	f2$ci[is.na(f2$ci)]=0
	f3 <- f2

	colours <- colorRampPalette(brewer.pal(6,"Greys"))(6)
	dodge <- position_dodge(width = 0.9)

	ggplot(f3, aes(x=mk, y=ch, fill=cth)) + 
	geom_bar(position=dodge, colour="black") + 
	geom_errorbar(width=.3, position=dodge, aes(ymin=ch-ci, ymax=ch+ci)) + 
	opts(title=titulo) +
	xlab("maxK") +
	ylab("Número de líderes") +
	scale_fill_manual(values=colours, 
		name="CThresh",
		breaks=c("p0.5", "p1.0", "p1.5", "p2.0"), 
		labels=c("0.5", "1.0", "1.5", "2.0") ) +
	scale_x_discrete(breaks=c("p1", "p2", "p3"),
					   labels=c("1","2","3")) +
	scale_y_continuous(expand=c(0,0), limits=c(0, 20)) +
	opts(aspect.ratio=1/1) +
	theme_bw() +
    opts(
		legend.justification=c(1,1),
		legend.position=c(1,1),
        legend.key = theme_blank()) + 
	opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
   	opts(aspect.ratio=1/1) +
	opts(axis.title.x = theme_text(vjust=-0.1, size = 18.5), axis.title.y = theme_text(size = 18.5, angle = 90), axis.text.x = theme_text(size=18.5*0.8), axis.text.y = theme_text(size=18.5*0.8)) + # more space below Xlabel
	ggsave(file=filename, height=6, width=6)

}

dv_quadro <- function () {
	
	#FILENAME
	filename=paste(basepath,"dv_quadro.pdf",sep="")
	
	f1<-f
	f2 <- summarySE(f1, measurevar="dv", groupvars=c("cth","int", "adap", "mk"))
	f2$ci[is.na(f2$ci)]=0
	desired_order <- c("p6", "p12", "p24", "p48", "p96")
	f3 <- f2
	f3$int  <- factor( as.character(f3$int), levels=desired_order )
	f3 <- f3[order(f3$int),]

	colours <- colorRampPalette(brewer.pal(6,"Greys"))(6)
	dodge <- position_dodge(width = 0.9)

	#rename everything
	levels(f3$adap) <- gsub("p0", "intervalo não adaptativo", levels(f3$adap))
	levels(f3$adap) <- gsub("p1", "intervalo adaptativo", levels(f3$adap))
	
	levels(f3$mk) <- gsub("p1", "maxK = 1", levels(f3$mk))
	levels(f3$mk) <- gsub("p2", "maxK = 2", levels(f3$mk))
	levels(f3$mk) <- gsub("p3", "maxK = 3", levels(f3$mk))
	
	
	ggplot(f3, aes(x=cth, y=dv, fill=int)) + 
	geom_bar(position=dodge, colour="black") + 
	facet_grid(adap ~ mk) + 
	geom_errorbar(width=.3, position=dodge, aes(ymin=dv-ci, ymax=dv+ci)) + 
	opts(title="") +
	xlab("CThresh") +
	ylab("Desvio padrão médio nas leitura dos agrupamentos") +
	scale_fill_manual(values=colours, 
		name="Intervalo\nbase",
		breaks=c("p6", "p12", "p24", "p48", "p96"), 
		labels=c("6.0s", "12.0s", "24.0s", "48.0s","96.0s") ) +
	scale_x_discrete(breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
					   labels=c("0.5","1.0","1.5", "2.0")) +
	scale_y_continuous(expand=c(0,0), limits=c(0, 0.9)) +
	theme_bw() +
	opts(
		legend.justification=c(0,1),
		legend.position="right",
	    legend.key = theme_blank()) + 
	#opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
	opts(aspect.ratio=1/1.5) +
	opts(axis.title.x = theme_text(vjust=-0.1)) +
	ggsave(file=filename, width=14.5, height=6.75)
}


amp_quadro <- function () {
	
	#FILENAME
	filename=paste(basepath,"amp_quadro.pdf",sep="")
	
	f1<-f
	f2 <- summarySE(f1, measurevar="amp", groupvars=c("cth","int", "adap", "mk"))
	f2$ci[is.na(f2$ci)]=0
	desired_order <- c("p6", "p12", "p24", "p48", "p96")
	f3 <- f2
	f3$int  <- factor( as.character(f3$int), levels=desired_order )
	f3 <- f3[order(f3$int),]

	colours <- colorRampPalette(brewer.pal(6,"Greys"))(6)
	dodge <- position_dodge(width = 0.9)

	#rename everything
	levels(f3$adap) <- gsub("p0", "intervalo ~adaptativo", levels(f3$adap))
	levels(f3$adap) <- gsub("p1", "intervalo adaptativo", levels(f3$adap))
	
	levels(f3$mk) <- gsub("p1", "maxK = 1", levels(f3$mk))
	levels(f3$mk) <- gsub("p2", "maxK = 2", levels(f3$mk))
	levels(f3$mk) <- gsub("p3", "maxK = 3", levels(f3$mk))
	
	# ggplot(f3, aes(x=cth, y=amp, fill=int)) + 
	# geom_bar(position=dodge, colour="black") + 
	# facet_grid(adap ~ mk) + 
	# geom_errorbar(width=.3, position=dodge, aes(ymin=amp-ci, ymax=amp+ci)) + 
	# opts(title="") +
	# xlab("CThresh") +
	# ylab("Amplitude média nas leitura dos agrupamentos") +
	# scale_fill_manual(values=colours, 
	# 	name="Intervalo\nbase",
	# 	breaks=c("p6", "p12", "p24", "p48", "p96"), 
	# 	labels=c("6.0s", "12.0s", "24.0s", "48.0s","96.0s") ) +
	# scale_x_discrete(breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
	# 				   labels=c("0.5","1.0","1.5", "2.0")) +
	# scale_y_continuous(expand=c(0,0), limits=c(0, 3.3)) +
	# theme_bw() +
	# opts(
	# 	legend.justification=c(0,1),
	# 	legend.position="right",
	#     legend.key = theme_blank()) + 
	# #opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
	# opts(aspect.ratio=1/1.5) +
	# opts(axis.title.x = theme_text(vjust=-0.1)) +
	# # opts(axis.title.x = theme_text(vjust=-0.1, size = 18.5), axis.title.y = theme_text(size = 18.5, angle = 90), axis.text.x = theme_text(size=18.5*0.8), axis.text.y = theme_text(size=18.5*0.8)) + # more space below Xlabel
	# ggsave(file=filename, width=14.5, height=6.75)
	
	ggplot(f3, aes(x=cth, y=amp, fill=int)) + 
		geom_bar(position=dodge, colour="black") + 
		facet_grid(adap ~ mk) + 
		geom_errorbar(width=.3, position=dodge, aes(ymin=amp-ci, ymax=amp+ci)) + 
		opts(title="") +
		xlab("CThresh") +
		ylab("Amplitude média nas leitura dos agrupamentos") +
		scale_fill_manual(values=colours, 
			name="Intervalo\nbase",
			breaks=c("p6", "p12", "p24", "p48", "p96"), 
			labels=c("6.0s", "12.0s", "24.0s", "48.0s","96.0s") ) +
		scale_x_discrete(breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
						   labels=c("0.5","1.0","1.5", "2.0")) +
		scale_y_continuous(expand=c(0,0), limits=c(0, 3.3)) +
		theme_bw() +
		opts(
			legend.justification=c(0,1),
			legend.position="right",
		    legend.key = theme_blank()) + 
		#opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
		opts(aspect.ratio=1/1.5) +
		opts(axis.title.x = theme_text(vjust=-0.1)) +
		# opts(axis.title.x = theme_text(vjust=-0.1, size = 18.5), axis.title.y = theme_text(size = 18.5, angle = 90), axis.text.x = theme_text(size=18.5*0.8), axis.text.y = theme_text(size=18.5*0.8)) + # more space below Xlabel
		ggsave(file=filename, width=10, height=4.75)
}


uc_vs_cth <- function() {

	#FILENAME
	filename=paste(basepath,"uc_vs_cth.pdf",sep="")
	# titulo="Número de agrupamentos formados com relação ao CThresh"
	titulo=""

	# f1 <- f[f$int=="p12" & f$mk=="p2" & f$adap=="p1",]
	f1 <- f

	f2 <- summarySE(f1, measurevar="uc", groupvars=c("cth"))
	f2$ci[is.na(f2$ci)]=0
	f3 <- f2

	dodge <- position_dodge(width = 0.9)

	ggplot(f3, aes(x=cth, y=uc, fill="Grey90")) + 
	geom_bar(width = 0.4, position = position_dodge(width=0.75), colour="black") + 
	geom_errorbar(width=.3, position=dodge, aes(ymin=uc-ci, ymax=uc+ci)) + 
	opts(title=titulo) +
	xlab("CThresh") +
	ylab("Número de agruamentos formados") +
	scale_fill_manual(values=c("#444444")) +
	scale_x_discrete(breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
					   labels=c("0.5","1.0","1.5", "2.0")) +
	scale_y_continuous(expand=c(0,0), limits=c(0, 15)) +
	opts(aspect.ratio=1/1) +
	theme_bw() +
    opts(
		legend.justification=c(0,1),
		legend.position="none",
        legend.key = theme_blank()) + 
	opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
   	opts(aspect.ratio=1/1) +
	opts(axis.title.x = theme_text(vjust=-0.1, size = 18.5), axis.title.y = theme_text(size = 18.5, angle = 90), axis.text.x = theme_text(size=18.5*0.8), axis.text.y = theme_text(size=18.5*0.8)) + # more space below Xlabel
	ggsave(file=filename, height=6, width=6)
	
}

ln_vs_cth <- function() {

	#FILENAME
	filename=paste(basepath,"ln_vs_cth.pdf",sep="")
	# titulo="Número de agrupamentos formados com relação ao CThresh"
	titulo=""

	# f1 <- f[f$int=="p12" & f$mk=="p2" & f$adap=="p1",]
	f1 <- f

	f2 <- summarySE(f1, measurevar="ln", groupvars=c("cth"))
	f2$ci[is.na(f2$ci)]=0
	f3 <- f2

	dodge <- position_dodge(width = 0.9)

	ggplot(f3, aes(x=cth, y=ln, fill="Grey90")) + 
	geom_bar(width = 0.4, position = position_dodge(width=0.75), colour="black") + 
	geom_errorbar(width=.3, position=dodge, aes(ymin=ln-ci, ymax=ln+ci)) + 
	opts(title=titulo) +
	xlab("CThresh") +
	ylab("Número de nós solitários") +
	scale_fill_manual(values=c("#444444")) +
	scale_x_discrete(breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
					   labels=c("0.5","1.0","1.5", "2.0")) +
	scale_y_continuous(expand=c(0,0), limits=c(0, 15)) +
	opts(aspect.ratio=1/1) +
	theme_bw() +
    opts(
		legend.justification=c(0,1),
		legend.position="none",
        legend.key = theme_blank()) + 
	opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
   	opts(aspect.ratio=1/1) +
	opts(axis.title.x = theme_text(vjust=-0.1, size = 18.5), axis.title.y = theme_text(size = 18.5, angle = 90), axis.text.x = theme_text(size=18.5*0.8), axis.text.y = theme_text(size=18.5*0.8)) + # more space below Xlabel
	ggsave(file=filename, height=6, width=6)
	
}

Xh_vs_cth_old <- function() {
	
	INT_PARAMS <- c("p6", "p12", "p24", "p48", "p96")
	ADAP_PARAMS <- c("p0", "p1")
	MK_PARAMS <- c("p1", "p2", "p3")

	for (pint in INT_PARAMS) {
		for (padap in ADAP_PARAMS) {
			for (pmk in MK_PARAMS) {

				#FILENAME
				filename=paste(basepath,"Xh_vs_cth",sep="")
				filename=paste(filename,"adap",sep=":")
				filename=paste(filename,substring(padap,2),sep=":")
				filename=paste(filename,"int",sep=":")
				filename=paste(filename,substring(pint,2),sep=":")
				filename=paste(filename,"mk",sep=":")
				filename=paste(filename,substring(pmk,2),sep=":")
				filename=paste(filename,"pdf",sep=".")

				titulo="Quantidade de nós por distância até o líder considerando\n"
				if (padap=="p1") {
				titulo=paste(titulo,"intervalo adaptativo ",sep="")
				} else {
					titulo=paste(titulo,"intervalo não adaptativo ",sep="")
				}
				titulo=paste(titulo,"com intervalo-base = ",sep="")
				titulo=paste(titulo,substring(pint,2),sep="")
				titulo=paste(titulo,".0s",sep="")
				titulo=paste(titulo,", maxK = ",sep="")
				titulo=paste(titulo,substring(pmk,2),sep="")


				# SELECT PARAMETERS
				f1 <- f[f$int==pint & f$adap==padap & f$mk==pmk,]

				f1h <- summarySE(f1, measurevar="X1h", groupvars=c("cth"))
				names(f1h)[names(f1h)=="ci"] <- "X1hci"
	
				f2h <- summarySE(f1, measurevar="X2h", groupvars=c("cth"))
				names(f2h)[names(f2h)=="ci"] <- "X2hci"
	
				f3h <- summarySE(f1, measurevar="X3h", groupvars=c("cth"))
				names(f3h)[names(f3h)=="ci"] <- "X3hci"
	
				f4h <- summarySE(f1, measurevar="X4h", groupvars=c("cth"))	
				names(f4h)[names(f4h)=="ci"] <- "X4hci"
	
				f1h$ci[is.na(f1h$X1hci)]=0
				f2h$ci[is.na(f2h$X2hci)]=0
				f3h$ci[is.na(f3h$X3hci)]=0
				f4h$ci[is.na(f4h$X4hci)]=0
	
				a <- merge(f1h, f2h, by=c("cth","cth"))
				b <- merge(f3h, f4h, by=c("cth","cth"))
				c <- merge(a, b, by=c("cth","cth"))
	
				# now I need to create my customized matrix -.-
				m <- matrix(ncol=4,byrow=TRUE)
				colnames(m) <- c("cth", "hops", "qtd", "qtdci")
	
				for(i in 1:nrow(c)) {
				    row <- c[i,]
				    m <- insertRow(m,1,c(row$cth,"1",row$X1h,row$X1hci)) 
					m <- insertRow(m,1,c(row$cth,"2",row$X2h,row$X2hci)) 
					m <- insertRow(m,1,c(row$cth,"3",row$X3h,row$X3hci)) 
					m <- insertRow(m,1,c(row$cth,"4",row$X4h,row$X4hci)) 
				}
	
				mode(m) <- "numeric" # not strings

				# normalize names so I can understand them..
				f3 <- as.data.frame(m)
				f3$cth[f3$cth==1]="p0.5"
				f3$cth[f3$cth==2]="p1.0"
				f3$cth[f3$cth==3]="p1.5"
				f3$cth[f3$cth==4]="p2.0"
	
				f3$hops[f3$hops==1]="p1"
				f3$hops[f3$hops==2]="p2"
				f3$hops[f3$hops==3]="p3"
				f3$hops[f3$hops==4]="p4"
	
				dodge <- position_dodge(width = 0.9)
				colours <- colorRampPalette(brewer.pal(6,"Greys"))(6)

				ggplot(f3, aes(x=cth, y=qtd, fill=hops)) + 
				geom_bar(position=dodge, colour="black") + 
				geom_errorbar(width=.3, position=dodge, aes(ymin=qtd-qtdci, ymax=qtd+qtdci)) + 
				opts(title=titulo) +
				xlab("CThresh") +
				ylab("Número de nós") +
				scale_fill_manual(values=colours, 
					name="Quantidade\nde saltos",
					breaks=c("p1", "p2", "p3", "p4"), 
					labels=c("1", "2", "3", "4") ) +
				scale_x_discrete(breaks=c("p0.5", "p1.0", "p1.5", "p2.0"),
								   labels=c("0.5","1.0","1.5", "2.0")) +
				scale_y_continuous(expand=c(0,0), limits=c(0, 35)) +
				theme_bw() +
			    opts(
					legend.justification=c(1,1),
					legend.position=c(0.89,0.89),
			        legend.key = theme_blank()) + 
				opts(legend.background = theme_rect(fill="gray98", size=.5, linetype="dotted")) +  
			   	opts(aspect.ratio=1/1) +
				opts(axis.title.x = theme_text(vjust=-0.1)) + # more space below Xlabel
				ggsave(file=filename)
			}
		}
	}
}




Xh_vs_cth <- function () {
	
	
	filename=paste(basepath,"Xh_vs_cth.pdf",sep="")
	
	# SET ALL IDS

	INT_PARAMS <- c("p6", "p12", "p24", "p48", "p96")
	CTH_PARAMS <- c("p0.5", "p1.0", "p1.5", "p2.0")
	MK_PARAMS <- c("p1", "p2", "p3")
	ADAP_PARAMS <-c("p1", "p0")


	for (padap in ADAP_PARAMS) {
		for (pint in INT_PARAMS) {
			for (pmk in MK_PARAMS) {
				for (pcth in CTH_PARAMS) {
					cname=""
					cname=paste(cname,"cth",sep="")
					cname=paste(cname,substring(pcth,2),sep="")
					cname=paste(cname,"int",sep="")
					cname=paste(cname,substring(pint,2),sep="")
					cname=paste(cname,"adap",sep="")
					cname=paste(cname,substring(padap,2),sep="")
					cname=paste(cname,"mk",sep="")
					cname=paste(cname,substring(pmk,2),sep="")

					print (cname)

					f$id[f$adap==padap & f$int==pint & f$mk==pmk & f$cth==pcth]=cname
				}
			}
		}
	}



	# PREPARE DATA FRAME

	col_names <- c("cname", "hops", "qtd", "int", "mk", "cth", "adap")
	f3f <- data.frame(rbind(c(NA,NA,NA,NA,NA,NA,NA)))
	names(f3f) <- col_names
	f3f <- f3f[-1,] # remove NAs

	desired_order <- c()

	for (padap in ADAP_PARAMS) {
		for (pint in INT_PARAMS) {
			for (pmk in MK_PARAMS) {

				cname=""
				cname=paste(cname,"MaxK ",sep="")
				cname=paste(cname,substring(pmk,2),sep="")
				cname=paste(cname,", int ",sep="")
				cname=paste(cname,substring(pint,2),sep="")
				cname=paste(cname,", adap ",sep="")
				cname=paste(cname,substring(padap,2),sep="")

				print(cname)

				desired_order <- c(desired_order,cname)



				f1 <- f[f$adap==padap & f$int==pint & f$mk==pmk,]

				f1h <- summarySE(f1, measurevar="X1h", groupvars=c("cth","mk","int","adap","id"))
				names(f1h)[names(f1h)=="ci"] <- "X1hci"

				f2h <- summarySE(f1, measurevar="X2h", groupvars=c("cth","mk","int","adap","id"))
				names(f2h)[names(f2h)=="ci"] <- "X2hci"

				f3h <- summarySE(f1, measurevar="X3h", groupvars=c("cth","mk","int","adap","id"))
				names(f3h)[names(f3h)=="ci"] <- "X3hci"

				f4h <- summarySE(f1, measurevar="X4h", groupvars=c("cth","mk","int","adap","id"))
				names(f4h)[names(f4h)=="ci"] <- "X4hci"

				f1h$ci[is.na(f1h$X1hci)]=0
				f2h$ci[is.na(f2h$X2hci)]=0
				f3h$ci[is.na(f3h$X3hci)]=0
				f4h$ci[is.na(f4h$X4hci)]=0


				a <- merge(f1h, f2h, by=c("id","id"))
				b <- merge(f3h, f4h, by=c("id","id"))
				c <- merge(a, b, by=c("id","id"))

				#order that fucking shit
				desired_order <- c("p6", "p12", "p24", "p48", "p96")
				c$int.x.x  <- factor( as.character(c$int.x.x), levels=desired_order )
				c <- c[order(c$int.x.x),]


				m <- matrix(ncol=7,byrow=TRUE)
				colnames(m) <- c("cname", "hops", "qtd", "int", "mk", "cth", "adap")	
				m <- m[-1,] # remove NAs

				for(i in 1:nrow(c)) {
				    row <- c[i,]

				    m <- insertRow(m,1,c(cname,"1",row$X1h,row$int.x.x,row$mk.x.x,row$cth.x.x,row$adap.x.x)) 
					m <- insertRow(m,1,c(cname,"2",row$X2h,row$int.x.x,row$mk.x.x,row$cth.x.x,row$adap.x.x)) 
					m <- insertRow(m,1,c(cname,"3",row$X3h,row$int.x.x,row$mk.x.x,row$cth.x.x,row$adap.x.x)) 
					m <- insertRow(m,1,c(cname,"4",row$X4h,row$int.x.x,row$mk.x.x,row$cth.x.x,row$adap.x.x)) 
				}

				# mode(m) <- "numeric" # not strings

				# normalize names so I can understand them..
				f3 <- as.data.frame(m)


				levels(f3$cth) <- gsub("1", "p0.5", levels(f3$cth))
				levels(f3$cth) <- gsub("2", "p1.0", levels(f3$cth))
				levels(f3$cth) <- gsub("3", "p1.5", levels(f3$cth))
				levels(f3$cth) <- gsub("4", "p2.0", levels(f3$cth))

				levels(f3$mk) <- gsub("1", "p1", levels(f3$mk))
				levels(f3$mk) <- gsub("2", "p2", levels(f3$mk))
				levels(f3$mk) <- gsub("3", "p3", levels(f3$mk))

				levels(f3$hops) <- gsub("1", "p1", levels(f3$hops))
				levels(f3$hops) <- gsub("2", "p2", levels(f3$hops))
				levels(f3$hops) <- gsub("3", "p3", levels(f3$hops))
				levels(f3$hops) <- gsub("4", "p4", levels(f3$hops))

				levels(f3$adap) <- gsub("1", "p0", levels(f3$adap))
				levels(f3$adap) <- gsub("2", "p1", levels(f3$adap))

				levels(f3$int) <- gsub("1", "p6", levels(f3$int))
				levels(f3$int) <- gsub("2", "p12", levels(f3$int))
				levels(f3$int) <- gsub("3", "p24", levels(f3$int))
				levels(f3$int) <- gsub("4", "p48", levels(f3$int))
				levels(f3$int) <- gsub("5", "p96", levels(f3$int))


				f3$qtd <- as.numeric(as.character(f3$qtd)) # convert quantities to numbers

				f3f <- merge(f3f,f3,all=TRUE,sort=TRUE)
			}
		}
	}

	# f3f$cname  <- factor( as.character(f3f$cname), levels=desired_order )
	# f3f <- f3f[order(f3f$cname),]

	# no lugar de adap é a coluna descritora da parada

	f3f <- f3f[f3f$hops != "p4",]
	f3f$hops <- factor(f3f$hops) # remove p4 factor for hops

	x_axis_params_qtd <- factor(f3f$cname)


	# ggplot (f3f, aes_now(x=x_axis_params_qtd, y=f3f$qtd)) +
	# geom_point(position = position_jitter(width = 0.175), alpha = 1, aes(size=hops, shape=cth)) + 
	# ylab("Quantidade de nós") +
	# xlab("Parâmetros") +
	# scale_size_discrete(name="Quantidade\nde saltos", breaks=c("p1","p2","p3"), labels=c("1","2","3")) +
	# scale_shape_manual(aes(name="CThresh"), values=c(19,0,2,5), breaks=c("p0.5","p1.0","p1.5", "p2.0"), labels=c("0.5","1.0","1.5", "2.0")) +
	# opts(aspect.ratio=1/3) + 
	# opts(axis.text.x=theme_text(angle=-40, hjust=0,vjust=1))
	
	ggplot (f3f, aes_now(x=x_axis_params_qtd, y=f3f$qtd)) +
		geom_point(position = position_jitter(width = 0.175), alpha = 0.9, aes(size=hops, color=cth), shape=10) + 
		ylab("Quantidade de nós") +
		xlab("Parâmetros") +
		scale_size_manual(name="Quantidade\nde saltos", values=c(1.5,3,4.5,6), breaks=c("p1","p2","p3"), labels=c("1","2","3")) +
		scale_color_manual(name="CThresh", values=c("#D7191C","#FDAE61","#9B4484","#2B83BA"), breaks=c("p0.5","p1.0","p1.5", "p2.0"), labels=c("0.5","1.0","1.5", "2.0")) +
		opts(aspect.ratio=1/3) + 
		opts(axis.text.x=theme_text(angle=-40, hjust=0,vjust=1)) +
		ggsave(file=filename,  width=12, height=5.14) 

}

# ch_duration_histogram <- function() {
# 	q <- read.table("/Users/fgielow/Dropbox/FACUL/Dissertacao/SIM_ANALYSIS/KHOPCA/GLOBAL_FINISH_TABLE_FOR_R_2", row.names=NULL, header=TRUE)
# 	GLOBAL_FINISH_TABLE_FOR_R_2
# }

histogram_duration_quadro <- function () {
	
	filename=paste(basepath,"histogram_duration_quadro.pdf",sep="")
	
	q <- read.table("/Users/fgielow/Dropbox/FACUL/Dissertacao/SIM_ANALYSIS/KHOPCA/GLOBAL_FINISH_TABLE_FOR_R_2", row.names=NULL, header=TRUE)
	
	# w <- q[q$cth!="p2.0",] #ignore cth 2.0
	
	w <- q
	
	e <- summarySE(w, measurevar="qtd", groupvars=c("dur", "cth", "int"))
		
	desired_order <- c("p6", "p12", "p24", "p48", "p96")
	f3 <- e
	f3$int  <- factor( as.character(f3$int), levels=desired_order )
	f3 <- f3[order(f3$int),]

	#rename everything
	levels(f3$int) <- gsub("p6", "intervalo de 6.0s", levels(f3$int))
	levels(f3$int) <- gsub("p12", "intervalo de 12.0s", levels(f3$int))
	levels(f3$int) <- gsub("p24", "intervalo de 24.0s", levels(f3$int))
	levels(f3$int) <- gsub("p48", "intervalo de 48.0s", levels(f3$int))
	levels(f3$int) <- gsub("p96", "intervalo de 96.0s", levels(f3$int))
	
	levels(f3$cth) <- gsub("p0.5", "CThresh = 0.5", levels(f3$cth))
	levels(f3$cth) <- gsub("p1.0", "CThresh = 1.0", levels(f3$cth))
	levels(f3$cth) <- gsub("p1.5", "CThresh = 1.5", levels(f3$cth))
	levels(f3$cth) <- gsub("p2.0", "CThresh = 2.0", levels(f3$cth))
	
	qplot(dur, data=f3, weight=qtd, geom="histogram", facets=cth~int) + 
	scale_y_continuous(expand=c(0,0), limits=c(0, 17.5)) +
	opts(title="") +
	xlab("Duração em número de turnos") +
	ylab("Número de líderes") +
	theme_bw() +
	opts(axis.title.x = theme_text(vjust=-0.1)) + # more space below Xlabel
	ggsave(file=filename, width=10, height=5.75)

}

histogram_duration_quadro_filtrado <- function () {
	
	filename=paste(basepath,"histogram_duration_quadro_cth_filtrado.pdf",sep="")
	
	q <- read.table("/Users/fgielow/Dropbox/FACUL/Dissertacao/SIM_ANALYSIS/KHOPCA/GLOBAL_FINISH_TABLE_FOR_R_2", row.names=NULL, header=TRUE)
	
	# w <- q[q$cth!="p2.0",] #ignore cth 2.0
	
	w <- q
	
	e <- summarySE(w, measurevar="qtd", groupvars=c("dur", "cth", "int"))
		
	desired_order <- c("p6", "p12", "p24", "p48", "p96")
	f3 <- e[e$cth=="p0.5",]
	f3$int  <- factor( as.character(f3$int), levels=desired_order )
	f3 <- f3[order(f3$int),]

	#rename everything
	levels(f3$int) <- gsub("p6", "intervalo de 6.0s", levels(f3$int))
	levels(f3$int) <- gsub("p12", "intervalo de 12.0s", levels(f3$int))
	levels(f3$int) <- gsub("p24", "intervalo de 24.0s", levels(f3$int))
	levels(f3$int) <- gsub("p48", "intervalo de 48.0s", levels(f3$int))
	levels(f3$int) <- gsub("p96", "intervalo de 96.0s", levels(f3$int))
	
	levels(f3$cth) <- gsub("p0.5", "CThresh = 0.5", levels(f3$cth))
	levels(f3$cth) <- gsub("p1.0", "CThresh = 1.0", levels(f3$cth))
	levels(f3$cth) <- gsub("p1.5", "CThresh = 1.5", levels(f3$cth))
	levels(f3$cth) <- gsub("p2.0", "CThresh = 2.0", levels(f3$cth))
	
	qplot(dur, data=f3, weight=qtd, geom="histogram", facets=cth~int) + 
	scale_y_continuous(expand=c(0,0), limits=c(0, 17.5)) +
	opts(title="") +
	xlab("Duração em número de turnos") +
	ylab("Número de líderes") +
	theme_bw() +
	opts(axis.title.x = theme_text(vjust=-0.1)) + # more space below Xlabel
	ggsave(file=filename, width=10, height=2.8)

}


# getAngle <- function(param) { ifelse (param=="qosb", 45, ifelse (param=="ir", 225, ifelse (param=="amp", -225, ifelse (param=="midDur", -45, 0)))) }
getAngle <- function(param) { ifelse (param=="qosb", -45, ifelse (param=="ir", -225, ifelse (param=="amp", 225, ifelse (param=="midDur", 45, 0)))) }

show_best_worst_scenarios <- function() {
	
	filename=paste(basepath,"best_worst_scenarios.pdf",sep="")
	
	q <- read.table("/Users/fgielow/Dropbox/FACUL/Dissertacao/SIM_ANALYSIS/KHOPCA/GLOBAL_FINISH_TABLE_FOR_R_2", row.names=NULL, header=TRUE)

	# SET ALL IDS

	INT_PARAMS <- c("p6", "p12", "p24", "p48", "p96")
	CTH_PARAMS <- c("p0.5", "p1.0", "p1.5", "p2.0")
	MK_PARAMS <- c("p1", "p2", "p3")
	ADAP_PARAMS <-c("p1", "p0")


	for (padap in ADAP_PARAMS) {
		for (pint in INT_PARAMS) {
			for (pmk in MK_PARAMS) {
				for (pcth in CTH_PARAMS) {
					cname=""
					cname=paste(cname,"cth",sep="")
					cname=paste(cname,substring(pcth,2),sep="")
					cname=paste(cname,"int",sep="")
					cname=paste(cname,substring(pint,2),sep="")
					cname=paste(cname,"adap",sep="")
					cname=paste(cname,substring(padap,2),sep="")
					cname=paste(cname,"mk",sep="")
					cname=paste(cname,substring(pmk,2),sep="")

					print (cname)

					f$id[f$adap==padap & f$int==pint & f$mk==pmk & f$cth==pcth]=cname
				}
			}
		}
	}


	for (padap in ADAP_PARAMS) {
		for (pint in INT_PARAMS) {
			for (pmk in MK_PARAMS) {
				for (pcth in CTH_PARAMS) {
					cname=""
					cname=paste(cname,"cth",sep="")
					cname=paste(cname,substring(pcth,2),sep="")
					cname=paste(cname,"int",sep="")
					cname=paste(cname,substring(pint,2),sep="")
					cname=paste(cname,"adap",sep="")
					cname=paste(cname,substring(padap,2),sep="")
					cname=paste(cname,"mk",sep="")
					cname=paste(cname,substring(pmk,2),sep="")

					print (cname)

					q$id[q$adap==padap & q$int==pint & q$mk==pmk & q$cth==pcth]=cname
				}
			}
		}
	}

	#PREPARE DUR TABLE
	# dur <- summarySE(f, measurevar="qosb", groupvars=c("cth","mk","int","adap","id"))


	e <- summarySE(q, measurevar="qtd", groupvars=c("dur", "cth", "int", "adap", "mk", "id"))

	mid <- matrix(ncol=2,byrow=TRUE)
	colnames(mid) <- c("id", "midDur")
	mid <- mid[-1,]

	for (padap in ADAP_PARAMS) {
		for (pint in INT_PARAMS) {
			for (pmk in MK_PARAMS) {
				for (pcth in CTH_PARAMS) {

					cname=""
					cname=paste(cname,"cth",sep="")
					cname=paste(cname,substring(pcth,2),sep="")
					cname=paste(cname,"int",sep="")
					cname=paste(cname,substring(pint,2),sep="")
					cname=paste(cname,"adap",sep="")
					cname=paste(cname,substring(padap,2),sep="")
					cname=paste(cname,"mk",sep="")
					cname=paste(cname,substring(pmk,2),sep="")

					print (cname)

					tmp <- e[e$adap==padap & e$int==pint & e$mk==pmk & e$cth==pcth,]
					sum=0
					for(i in 1:nrow(tmp)) {
					    row <- tmp[i,]
						sum = sum + row$qtd * row$dur
					}
					midDur = sum / nrow(tmp)

					mid <- insertRow(mid,1,c(cname,midDur))

				}
			}
		}
	}


	# GET OTHER TABLES

	# qosb accumulates till end!
	qosb <- summarySE(f[f$time==1190,], measurevar="qosb", groupvars=c("cth","mk","int","adap","id"))
	ir <- summarySE(f, measurevar="ir", groupvars=c("cth","mk","int","adap","id"))
	amp <- summarySE(f, measurevar="amp", groupvars=c("cth","mk","int","adap","id"))

	a <- merge(qosb, ir, by=c("id"))
	b <- merge(amp, mid, by=c("id"))
	c <- merge(a, b, by=c("id"))

	c <- c[,c('id','cth','mk','int','adap','qosb','amp','ir','midDur')] #select only necessary cols

	desired_order <- c("p6", "p12", "p24", "p48", "p96")
	c$int  <- factor( as.character(c$int), levels=desired_order )
	c <- c[order(c$int),]


	# NORMALIZE FUCKING VALUES NOW FUCK

	minQosb = min(c$qosb)
	maxQosb = max(c$qosb)
	# c$qosb=(2*maxQosb - c$qosb)/(2*maxQosb - minQosb)
	c$qosb = (3/3 - ((2/3)*(c$qosb-minQosb)/(maxQosb-minQosb))) # [ 0.33, 1.0 ]

	minIr = min(c$ir)
	maxIr = max(c$ir)
	# c$ir=(2*maxIr - c$ir)/(2*maxIr - minIr)
	c$ir = (3/3 - ((2/3)*(c$ir-minIr)/(maxIr-minIr))) # [ 0.33, 1.0 ]

	minAmp = min(c$amp)
	maxAmp = max(c$amp)
	# c$amp=(2*maxAmp - c$amp)/(2*maxAmp - minAmp)
	c$amp = (3/3 - ((2/3)*(c$amp-minAmp)/(maxAmp-minAmp))) # [ 0.33, 1.0 ]


	c$midDur <- as.numeric(as.character(c$midDur))
	minMid = min(c$midDur)
	maxMid = max(c$midDur)
	# c$midDur=c$midDur / maxMid
	c$midDur = (1/3 + ((2/3)*(c$midDur-minMid)/(maxMid-minMid))) # [ 0.33, 1.0 ]

	# only highest and lowest metrics
	
	c$points <- rowSums(c[,c("qosb","amp","ir","midDur")])
	d <- c[with(c,order(-points)),] # order by points
	
	#d <- c[c$midDur == min(c$midDur) | c$midDur == max(c$midDur) | c$amp == max(c$amp) | c$amp == min(c$amp) | c$ir == min(c$ir) | c$ir == max(c$ir) | c$qosb == max(c$qosb) | c$qosb == min(c$qosb)  ,]
	d <- head(d,n=21L) # 21 isntances

	# DENORMALIZE FUCKING C NOW FUCK

	m <- matrix(ncol=7,byrow=TRUE)
	colnames(m) <- c("cth", "adap", "mk", "int", "attr", "v", "points")
	m <- m[-1,] # remove NAs

	for(i in 1:nrow(d)) {
	    row <- d[i,]
	    m <- insertRow(m,nrow(m)+1,c(row$cth, row$adap, row$mk, row$int, "qosb", row$qosb, row$points))
		m <- insertRow(m,nrow(m)+1,c(row$cth, row$adap, row$mk, row$int, "amp", row$amp, row$points))
		m <- insertRow(m,nrow(m)+1,c(row$cth, row$adap, row$mk, row$int, "ir", row$ir, row$points))
		m <- insertRow(m,nrow(m)+1,c(row$cth, row$adap, row$mk, row$int, "midDur", row$midDur, row$points))
	}

	f3 <- as.data.frame(m)

	levels(f3$cth) <- gsub("^1$", "\ncTh0.5", levels(f3$cth))
	levels(f3$cth) <- gsub("^2$", "\ncTh1.0", levels(f3$cth))
	levels(f3$cth) <- gsub("^3$", "\ncTh1.5", levels(f3$cth))
	levels(f3$cth) <- gsub("^4$", "\ncTh2.0", levels(f3$cth))

	levels(f3$mk) <- gsub("^1$", "mk1", levels(f3$mk))
	levels(f3$mk) <- gsub("^2$", "mk2", levels(f3$mk))
	levels(f3$mk) <- gsub("^3$", "mk3", levels(f3$mk)) 

	levels(f3$adap) <- gsub("^1$", "~adap", levels(f3$adap))
	levels(f3$adap) <- gsub("^2$", "adap", levels(f3$adap))

	levels(f3$int) <- gsub("^1$", "int6.0", levels(f3$int))
	levels(f3$int) <- gsub("^2$", "int12.0", levels(f3$int))
	levels(f3$int) <- gsub("^3$", "int24.0", levels(f3$int))
	levels(f3$int) <- gsub("^4$", "int48.0", levels(f3$int))
	levels(f3$int) <- gsub("^5$", "int96.0", levels(f3$int))

	f3$v <- as.numeric(as.character(f3$v)) # convert quantities to numbers
	f3$points <- as.numeric(as.character(f3$points)) # convert quantities to numbers
	
	f3$lbl <- paste(paste(paste(f3$adap, f3$int, sep=", "), f3$cth, sep=","), f3$mk, sep=", ")
	
	f3$lbl  <- factor( f3$lbl, levels=unique(f3$lbl))
	f3 <- f3[order(f3$lbl),]
	
	# f3$params  <- factor( as.character(f3$params), levels=desired_order )
	# f3 <- f3[order(f3$params),]
	
	# ggplot(f3, aes(factor(attr), v)) + 
	# geom_bar( aes(width=1, fill = factor(attr)), position = "stack",  stat = "identity") + 
	# coord_polar() + 
	# scale_fill_manual(values=colorRampPalette(brewer.pal(9,"Greys"))(4)) +
	# labs(x = "", y = "") +
	# opts(legend.position = "none", axis.text.y = theme_blank(), axis.ticks = theme_blank()) + 
	# opts(axis.text.x = theme_text(vjust=-0.5, angle=getAngle(attr))) +
	# facet_wrap( ~ lbl, nrow=3) +
	ggplot(f3, aes(factor(attr), v)) + 
		geom_bar( aes(width=1, fill = factor(attr)), position = "stack",  stat = "identity") + 
		coord_polar() + 
		scale_fill_manual(values=colorRampPalette(brewer.pal(9,"Greys"))(4)) +
		labs(x = "", y = "") +
		opts(legend.position = "none", axis.text.y = theme_blank(), axis.ticks = theme_blank()) + 
		opts(axis.text.x = theme_text(vjust=-0.5, angle=getAngle(f3$attr))) +
		facet_wrap( ~ lbl, nrow=3) +
		ggsave(file=filename, width=11,height=6)
}

nodes_id_top <- function() {
	x<-xmlParse("/opt/mote_locs.xml")
	t<-xmlToDataFrame(x)
	
	t$x <- as.numeric(as.character(t$x))
	t$y <- as.numeric(as.character(t$y))
	
	ggplot(t, aes(x,y)) + geom_point(size=3.5) +opts(aspect.ratio=35/42.5)
}




#Xh_vs_cth()
#ir_vs_mk_ch_int()
#ch_vs_cth_ch_mk()
#uc_vs_cth() #change somehow
#ln_vs_cth() #change somehow
#histogram_duration_quadro()
histogram_duration_quadro_filtrado()
#ir_vs_time_scatterplot()
#beacon_vs_time_scatterplot()
#dv_quadro()
#amp_quadro()
#show_best_worst_scenarios()
