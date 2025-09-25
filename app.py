import streamlit as st
import pandas as pd
import time
import plotly.graph_objects as go

st.set_page_config(
    page_title="TempTracker",
    page_icon="🌡️"
)

# Global variables definitions
if 'patient_saved' not in st.session_state:
    st.session_state.patient_saved = False
if "user_email" not in st.session_state:
    st.session_state.user_email = None
if 'logged_in' not in st.session_state:
    st.session_state.logged_in = False
if 'codigo' not in st.session_state:
    st.session_state.codigo = None
if 'buscar' not in st.session_state:
    st.session_state.buscar= False
if 'graficar' not in st.session_state:
    st.session_state.graficar= False

USER_DATA = {
    "valu": "valu",
    "sofi": "sofi",
}



def login_page() -> None:
    st.image(r'./bonemarrow_background.jpeg', use_container_width=True)
    st.title("Inicio de Sesión")
    email = st.text_input("Email")
    password = st.text_input("Contraseña", type="password")
    if st.button("Iniciar Sesión"):
        if email in USER_DATA and USER_DATA[email] == password:
            st.session_state.logged_in = True
            st.session_state.user_email = email
            st.success("¡Inicio de sesión exitoso!")
            time.sleep(2)
            st.rerun()
        else:
            st.error("Email o contraseña incorrectos")


def menu_page():
    st.sidebar.write(f"**Bienvenido**: {st.session_state.user_email}")
    if st.sidebar.button("Cerrar Sesión"):
        st.session_state.logged_in = False
        st.rerun()

    with st.sidebar:
        st.subheader("Finalidad")
        st.markdown("El objetivo de esta página es facilitar la visualización de los datos al llegar al destino. Se realiza una representación gráfica de la evolución temporal de la temperatura del interior de la cámara junto con métricas relevantes para evaluar la viabilidad celular de la muestra.")
        st.markdown('<h1 style="color:#a01545;">📈 Análisis tiempo temp</h1>', unsafe_allow_html=True)
        uploaded_file= st.sidebar.file_uploader("Seleccione el archivo que quiera graficar:", type = ["csv"])
        if st.sidebar.button("Graficar"):
            st.session_state.graficar= True

    
    if st.session_state.graficar:
        st.header(f"Gráfico del documento {uploaded_file.name}" )
        df = pd.read_csv(uploaded_file, sep=",", encoding="utf-8", header=None)
        df.columns = ['Tiempo', 'Temperatura', 'Puerta']

        show_df = st.checkbox("Seleccione para visualizar los datos en formato tabla.")

        if show_df:
            st.dataframe(df) # solo muestra el df, lo podemos sacar
                    
        unidad_tiempo = st.radio("Tiempo de escala:", ["Segundos", "Minutos", "Horas"], horizontal=True)
        if unidad_tiempo == "Minutos":
            df['Tiempo'] = df['Tiempo'] / 60
            etiqueta_tiempo = "Tiempo [min]"
        elif unidad_tiempo == "Horas":
            df['Tiempo'] = df['Tiempo'] / 3600
            etiqueta_tiempo = "Tiempo [hs]"
        else:
            etiqueta_tiempo = "Tiempo [s]"
      

        fig = go.Figure()

        # Banda verde: rango seguro de temperatura (2–8 ºC)
        fig.add_shape(
            type="rect",
            x0=df['Tiempo'].min(),
            x1=df['Tiempo'].max(),
            y0=2,
            y1=8,
            fillcolor="green",
            opacity=0.2,
            layer="below",
            line_width=0
        )

        # Línea de temperatura
        fig.add_trace(go.Scatter(
            x=df['Tiempo'],
            y=df['Temperatura'],
            mode='lines',
            name='Temperatura [ºC]',
            line=dict(color='#a1677b')
        ))

        # Puntos de puerta abierta
        df_abierta = df[df["Puerta"] == 1]

        fig.add_trace(go.Scatter(
            x=df_abierta["Tiempo"],
            y=df_abierta["Temperatura"],  # los ponemos a la misma altura que la temp
            mode="markers",
            name="Puerta abierta",
            marker=dict(color="red", size=10, symbol="x")
        ))

        # Configuración de ejes y layout
        fig.update_layout(
            title="Evolución temporal de la temperatura.",
            xaxis_title=etiqueta_tiempo,
            yaxis_title="Temperatura [ºC]",
            height=400,
            width=700
)

       
        st.plotly_chart(fig, use_container_width=True)
        st.info('La **banda verde** muestra de manera visual si la temperatura se encuentra dentro del rango seguro (2-8ºC)', icon="ℹ️")

        # estadisticas
        st.subheader("Resumen del trayecto")
        temp_min = df['Temperatura'].min()
        temp_inicial = df['Temperatura'].iloc[0]
        temp_avg = df['Temperatura'].mean()
        tiempo_fuera_rango = df[(df['Temperatura'] < 2) | (df['Temperatura'] > 8)].shape[0] *3/60
        duracion_total = df.shape[0]*3 /60  # en mins, tenemos 3 segundos entre muestras
        aperturas = (df['Puerta'].shift(1) == 0) & (df['Puerta'] == 1) #da true cuando pasa de 0 a 1
        num_aperturas = aperturas.sum()



        idx_inicio_seguro = df[df['Temperatura'].between(2, 8)].first_valid_index()

        if idx_inicio_seguro is not None:
            df_desde_seguro = df.loc[idx_inicio_seguro:]
            temp_avg_desde_seguro = df_desde_seguro['Temperatura'].mean()
        else:
            temp_avg_desde_seguro = None  # nunca estuvo en rango


        col1, col2 = st.columns(2)
        with col1:
            st.metric("Temperatura inical", f"{temp_inicial:.2f} °C")
            st.metric("Temperatura mínima", f"{temp_min:.2f} °C")
            if temp_avg_desde_seguro is not None:
                st.metric("Promedio dentro del rango", f"{temp_avg_desde_seguro:.2f} °C")
            else:
                st.metric("Promedio dentro del rango", "No entró al rango")
        with col2:
            st.metric("Tiempo fuera de rango", f"{tiempo_fuera_rango} min")
            st.metric("Duración total", f"{duracion_total:.2f} min")
            st.metric("Nº veces que se abrió la puerta", num_aperturas)

       
def main():
    if st.session_state.logged_in:
        menu_page()

    else:
        login_page()

main()