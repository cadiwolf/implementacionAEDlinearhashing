# Linear Hashing Frontend

Este proyecto es una interfaz de usuario moderna construida con [Next.js](https://nextjs.org) y [Tailwind CSS](https://tailwindcss.com) para interactuar con una implementación de **Linear Hashing** en C++.

El objetivo es proporcionar una forma visual y sencilla de probar las operaciones CRUD (Crear, Leer, Actualizar, Eliminar) sobre la estructura de datos.

## Funcionalidades

La aplicación permite realizar las siguientes operaciones contra el backend:

1.  **Insertar Registro**: Formulario para agregar pares clave-valor (`key`, `value`).
2.  **Buscar Registro**: Búsqueda por clave específica. Muestra el valor si existe y permite eliminarlo directamente.
3.  **Listar Registros**: Tabla que muestra todos los registros almacenados actualmente en la estructura.
4.  **Eliminar Registro**: Acción para borrar un registro específico por su clave.

> **Nota**: La aplicación se actualiza automáticamente después de cada operación para reflejar el estado actual de la base de datos.

## Requisitos Previos

- **Backend C++**: Debe estar ejecutándose en `http://localhost:8080` (o el puerto configurado).
- **Node.js**: Versión 18 o superior.

## Configuración

Si tu backend corre en un puerto diferente a `8080`, edita la constante `API_URL` en el archivo `app/page.tsx`:

```typescript
const API_URL = "http://localhost:TU_PUERTO";
```

## Ejecución

1.  Instala las dependencias:

    ```bash
    npm install
    ```

2.  Inicia el servidor de desarrollo:

    ```bash
    npm run dev
    ```

3.  Abre [http://localhost:3000](http://localhost:3000) en tu navegador.

## Estructura del Proyecto

La estructura principal del código es la siguiente:

- **`app/page.tsx`**: Contiene toda la lógica de la aplicación y la interfaz de usuario.
    - Manejo de estado con React Hooks (`useState`, `useEffect`, `useCallback`).
    - Funciones `fetch` para comunicarse con la API REST del backend.
    - Componentes UI para formularios, tablas y mensajes de estado.
- **`app/layout.tsx`**: Define la estructura base HTML y metadatos de la página.
- **`app/globals.css`**: Estilos globales y configuración de Tailwind CSS.
- **`public/`**: Archivos estáticos (imágenes, iconos).

## API Esperada

El frontend espera que el backend exponga los siguientes endpoints REST:

- `POST /records`: Insertar `{ key, value }`.
- `GET /records`: Obtener lista `{ count, records: [] }`.
- `GET /records/:key`: Obtener un registro específico.
- `DELETE /records/:key`: Eliminar un registro.
